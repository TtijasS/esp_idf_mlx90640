#include "mlx90640.h"

/**
 * @brief Initial MPU6050 setup.
 *
 * Set up the MPU6050 registers for the first time. MPU is set to wake up, filter freq is set to 5Hz,
 * accelerometer to +-8g full scale range and gyroscope to +-1000 deg/s full scale range.
 * The FIFO buffer is set to store accelerometer and gyroscope data.
 *
 * @param i2c_buffer: struct with write_buffer, read_buffer
 * @return bool; true if the setup was successful, false otherwise
 */
bool mlx_initial_setup(i2c_buffer_type *i2c_buffer)
{
    // uint16_t data = 0;

    ESP_LOGI(TAG, "Setting up MLX90640");

    mlx_configure_register(i2c_buffer, MLX_CTLR_REG_1, 0x0001, true);
    mlx_configure_register(i2c_buffer, MLX_CTLR_REG_1, 0x0004, false);

    return true;
}

/**
 * @brief Modify the specific register bits (16-bit register).
 *
 * The function reads the register value, modifies the bits and writes the new value back to the register.
 * If the clear flag is set to true, the bits are cleared, otherwise they are set.
 *
 * @param i2c_buffer: struct with read_buffer, write_buffer
 * @param reg_addr: register address (uint16_t register address)
 * @param bit_mask: bit mask of selected bits (1 = selected, 0 = not selected)
 * @param set: set (true) or clear (false) the selected bits
 * @return bool: true if successfull, otherwise false
 */
bool mlx_configure_register(i2c_buffer_type *i2c_buffer, uint16_t reg_addr, uint16_t bit_mask, bool set)
{
    ESP_LOGI(TAG, "Setting register 0x%04X\n\t{", reg_addr);
    static uint16_t tmp_data = 0;
    if (i2c_buffer == NULL)
    {
        ESP_LOGI(TAG, "Aborting mlx_configure_register, i2c_buffer is NULL");
        return false;
    }

    if (!mlx_read_register(i2c_buffer, reg_addr, &tmp_data))
    {
        ESP_LOGI(TAG, "Failed to read register");
        return false;
    }

    ESP_LOGI(TAG, "Initial data:  0x%04X", tmp_data);
    if (set)
        tmp_data |= bit_mask; // set the bits with 1
    else
        tmp_data &= ~bit_mask; // clear the bits with 1

    ESP_LOGI(TAG, "Modified data: 0x%04X", tmp_data);

    // Prepare the write buffer
    i2c_buffer->write_buffer[0] = reg_addr >> 8;
    i2c_buffer->write_buffer[1] = reg_addr & 0xFF;
    i2c_buffer->write_buffer[2] = tmp_data >> 8;
    i2c_buffer->write_buffer[3] = tmp_data & 0xFF;
    ESP_LOGI(TAG, "Writing: 0x%02X-%02X-%02X-%02X", i2c_buffer->write_buffer[0], i2c_buffer->write_buffer[1], i2c_buffer->write_buffer[2], i2c_buffer->write_buffer[3]);
    // Transmit the data and return if the transmission was successful
    if (!mlx_transmit(i2c_buffer, 4))
    {
        ESP_LOGI(TAG, "Failed to transmit new config");
        return false;
    }

    uint16_t checkup_reading = 0;
    if (!mlx_read_register(i2c_buffer, reg_addr, &checkup_reading))
    {
        ESP_LOGI(TAG, "Failed to read register on checkup");
        return false;
    }

    if (tmp_data != checkup_reading)
    {
        ESP_LOGI(TAG, "Failed to set the register on checkup");
        ESP_LOGI(TAG, "Expected: 0x%04X", tmp_data);
        ESP_LOGI(TAG, "Received: 0x%04X\n\t}", checkup_reading);
        return false;
    }
    else
    {
        ESP_LOGI(TAG, "Register set successfully");
        ESP_LOGI(TAG, "Expected: 0x%04X", tmp_data);
        ESP_LOGI(TAG, "Received: 0x%04X\n\t}", checkup_reading);
        return true;
    }
}

/**
 * @brief Read the specific register value into a 16-bit variable.
 *
 * The function reads the register value and stores it into the data variable.
 *
 * @param i2c_buffer: struct with read_buffer, write_buffer
 * @param reg_addr: register address
 * @param data: pointer to the variable where the register value will be stored
 * @return true
 * @return false
 */
bool mlx_read_register(i2c_buffer_type *i2c_buffer, uint16_t reg_addr, uint16_t *data)
{
    if (data == NULL)
    {
        ESP_LOGI(TAG, "Aborting mlx_read_register, data is NULL");
        return NULL;
    }
    i2c_buffer->write_buffer[0] = reg_addr >> 8;
    i2c_buffer->write_buffer[1] = reg_addr & 0xFF;
    if (!mlx_transmit_receive(i2c_buffer, 2, 2))
    {
        ESP_LOGI(TAG, "Failed to read register");
        return false;
    }

    *data = (i2c_buffer->read_buffer[0] << 8) | i2c_buffer->read_buffer[1];
    return true;
}

/**
 * @brief Transmit MPU register data and read it's value.
 *
 * MCU_write -> REG_ADDR -> MCU_read <- REG_VALUE(s)
 *
 * Before transmision, you have to first setup the write_buffer
 * i2c_buffer.write_buffer[write_buf_size]
 * [0] - register address
 *
 * To read multiple values, set the read_buf_size to greater than 1.
 * Reading multiple values usually means reading subsequent registers.
 *
 * @param i2c_buffer: struct with read_buffer, write_buffer
 * @param write_buf_size: i2c_buffer.wirte_buffer size (how many values to transmit)
 * @param read_buf_size: i2c_buffer.read_buffer size (how many values to receive)
 *
 * @return bool: true if successfull, otherwise false
 */
bool mlx_transmit_receive(i2c_buffer_type *i2c_buffer, uint8_t write_buf_size, uint16_t read_buf_size)
{
    if (write_buf_size > sizeof(i2c_buffer->write_buffer))
    {
        ESP_LOGI(TAG, "The allocated i2c_buffer.write_buffer size is smaller than %d", write_buf_size);
        return false;
    }
    if (read_buf_size > sizeof(i2c_buffer->read_buffer))
    {
        ESP_LOGI(TAG, "The allocated i2c_buffer.read_buffer size is smaller than %d", read_buf_size);
        return false;
    }
    ESP_ERROR_CHECK(i2c_master_transmit_receive(master_dev_handle, i2c_buffer->write_buffer, write_buf_size, i2c_buffer->read_buffer, read_buf_size, I2C_TIMEOUT_MS));
    return true;
}

/**
 * @brief Write data to specific MPU6050 register
 *
 * MCU_write -> REG_ADDR -> REG_VALUE
 *
 * Before transmision, you have to first setup the write_buffer
 * i2c_buffer->write_buffer[I2C_WRITE_BUFF_SIZE]
 * [0] - register address
 * [1] - data to write
 *
 * @param i2c_buffer: struct with read_buffer, write_buffer
 * @param write_buf_size: i2c_buffer.wirte_buffer size (how many values to transmit)
 *
 * @return bool: true if successfull, otherwise false
 */
bool mlx_transmit(i2c_buffer_type *i2c_buffer, uint8_t write_buf_size)
{
    if (write_buf_size > sizeof(i2c_buffer->write_buffer))
    {
        ESP_LOGI(TAG, "The allocated i2c_buffer.write_buffer size is smaller than %d", write_buf_size);
        return false;
    }
    ESP_ERROR_CHECK(i2c_master_transmit(master_dev_handle, i2c_buffer->write_buffer, write_buf_size, I2C_TIMEOUT_MS));
    return true;
}

/**
 * @brief Read the data from the MLX90640 RAM.
 *
 * The function reads the data from the MLX90640 RAM register and stores it into the read_buffer.
 * You have to check the status register to see which page is active.
 *
 * @param i2c_buffer: struct with read_buffer, write_buffer
 * @return true if successfull, otherwise false
 */
bool mlx_read_ram_data(i2c_buffer_type *i2c_buffer)
{
    i2c_buffer->write_buffer[0] = MLX_RAM_REG >> 8;
    i2c_buffer->write_buffer[1] = MLX_RAM_REG & 0xFF;
    return mlx_transmit_receive(i2c_buffer, 2, 768);
}

/**
 * @brief
 *
 * @param i2c_buffer
 * @param mlx_data
 * @return true
 * @return false
 */
bool mlx_read_eeprom_dump(i2c_buffer_type *i2c_buffer, mlx_data_type *mlx_data)
{
    uint8_t eeprom_tmp_dump[1664] = {0};
    i2c_buffer->write_buffer[0] = MLX_EEPROM_DUMP_REG >> 8;
    i2c_buffer->write_buffer[1] = MLX_EEPROM_DUMP_REG & 0xFF;
    ESP_ERROR_CHECK(i2c_master_transmit_receive(master_dev_handle, i2c_buffer->write_buffer, 2, eeprom_tmp_dump, 1664, I2C_TIMEOUT_MS));
}

bool mlx_extract_eeprom_dump(mlx_data_type *mlx_data, uint8_t *eeprom_dmp)
{
    if (eeprom_dmp == NULL)
    {
        ESP_LOGI(TAG, "Aborting mlx_extract_eeprom_dump, eeprom_dmp is NULL");
        return false;
    }
    if (mlx_data == NULL)
    {
        ESP_LOGI(TAG, "Aborting mlx_extract_eeprom_dump, mlx_data is NULL");
        return false;
    }

    mlx_data->gain = (eeprom_dmp[0] << 8) | eeprom_dmp[1];
    mlx_data->vdd_25 = (eeprom_dmp[2] << 8) | eeprom_dmp[3];
    mlx_data->ptat_25 = (eeprom_dmp[4] << 8) | eeprom_dmp[5];
    mlx_data->kptat = (eeprom_dmp[6] << 8) | eeprom_dmp[7];
    mlx_data->tgc = (eeprom_dmp[8] << 8) | eeprom_dmp[9];
    mlx_data->sens_kv = (eeprom_dmp[10] << 8) | eeprom_dmp[11];
    mlx_data->sens_kta = (eeprom_dmp[12] << 8) | eeprom_dmp[13];
    mlx_data->ks_to_1 = (eeprom_dmp[14] << 8) | eeprom_dmp[15];
    mlx_data->ks_to_2 = (eeprom_dmp[16] << 8) | eeprom_dmp[17];
    mlx_data->ks_to_3 = (eeprom_dmp[18] << 8) | eeprom_dmp[19];
    mlx_data->ks_to_4 = (eeprom_dmp[20] << 8) | eeprom_dmp[21];
    mlx_data->cp_1 = (eeprom_dmp[22] << 8) | eeprom_dmp[23];
    mlx_data->cp_2 = (eeprom_dmp[24] << 8) | eeprom_dmp[25];

    for (int i = 26; i < 1664; ++i)
    {
        mlx_data->pix_offsets[i] = (eeprom_dmp[26 + i * 2] << 8) | eeprom_dmp[27 + i * 2];
    }
    return true;
}