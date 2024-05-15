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
    uint16_t data = 0;

    mlx_configure_register(i2c_buffer, MLX_STAT_REG, 0x0000, true);
    mlx_read_register(i2c_buffer, MLX_STAT_REG, &data);
    ESP_LOGI(TAG, "Status register: 0x%04X", data);

    mlx_configure_register(i2c_buffer, MLX_STAT_REG, 0x0010, true);
    mlx_read_register(i2c_buffer, MLX_STAT_REG, &data);
    ESP_LOGI(TAG, "Status register: 0x%04X", data);

    mlx_configure_register(i2c_buffer, MLX_STAT_REG, 0x0010, false);
    mlx_read_register(i2c_buffer, MLX_STAT_REG, &data);
    ESP_LOGI(TAG, "Status register: 0x%04X", data);

    mlx_configure_register(i2c_buffer, MLX_STAT_REG, 0x0010, true);
    mlx_read_register(i2c_buffer, MLX_STAT_REG, &data);
    ESP_LOGI(TAG, "Status register: 0x%04X", data);

    mlx_configure_register(i2c_buffer, MLX_STAT_REG, 0x0008, true);
    mlx_read_register(i2c_buffer, MLX_STAT_REG, &data);
    ESP_LOGI(TAG, "Status register: 0x%04X", data);

    mlx_configure_register(i2c_buffer, MLX_STAT_REG, 0x0018, false);
    mlx_read_register(i2c_buffer, MLX_STAT_REG, &data);
    ESP_LOGI(TAG, "Status register: 0x%04X", data);

    mlx_configure_register(i2c_buffer, MLX_STAT_REG, 0x0018, true);
    mlx_read_register(i2c_buffer, MLX_STAT_REG, &data);
    ESP_LOGI(TAG, "Status register: 0x%04X", data);

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
    if (i2c_buffer == NULL)
    {
        ESP_LOGI(TAG, "Aborting mlx_configure_register, i2c_buffer is NULL");
        return false;
    }
    i2c_buffer->write_buffer[0] = reg_addr >> 8;
    i2c_buffer->write_buffer[1] = reg_addr & 0xFF;
    if (!mlx_transmit_receive(i2c_buffer, 2, 2))
    {
        ESP_LOGI(TAG, "Failed to read register");
        return false;
    }

    uint16_t modified_data = (i2c_buffer->read_buffer[0] << 8) | i2c_buffer->read_buffer[1];
    ESP_LOGI(TAG, "Initial data: 0x%04X", modified_data);
    if (set)
        modified_data |= bit_mask; // set the bits with 1
    else
        modified_data &= ~bit_mask; // clear the bits with 1

    ESP_LOGI(TAG, "Modified data: 0x%04X", modified_data);

    // Prepare the write buffer
    i2c_buffer->write_buffer[0] = reg_addr >> 8;
    i2c_buffer->write_buffer[1] = reg_addr & 0xFF;
    i2c_buffer->write_buffer[2] = modified_data >> 8;
    i2c_buffer->write_buffer[3] = modified_data & 0xFF;
    ESP_LOGI(TAG, "Write buffer: 0x%02X 0x%02X 0x%02X 0x%02X", i2c_buffer->write_buffer[0], i2c_buffer->write_buffer[1], i2c_buffer->write_buffer[2], i2c_buffer->write_buffer[3]);
    // Transmit the data and return if the transmission was successful
    return mlx_transmit(i2c_buffer, 4);
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