#include "mlx90640_i2c_driver.h"
#include "mlx90640_api.h"

// Define the I2C master bus configuration
const i2c_master_bus_config_t master_bus_config = {
	.i2c_port = I2C_NUM_0,
	.sda_io_num = I2C_SDA_IO,
	.scl_io_num = I2C_SCL_IO,
	.clk_source = I2C_CLK_SRC_DEFAULT,
	.glitch_ignore_cnt = 7,
	.flags.enable_internal_pullup = true};

// Define the I2C device configuration
const i2c_device_config_t master_device_config = {
	.dev_addr_length = I2C_ADDR_BIT_LEN_7,
	.device_address = MLX90640_SLAVE_ADR,
	.scl_speed_hz = I2C_FREQ_HZ};

// Initialize handles (can be allocated or further defined elsewhere)
i2c_master_bus_handle_t master_bus_handle;
i2c_master_dev_handle_t master_dev_handle;

/**
 * @brief Initialize the I2C bus
 *
 * Add new master bus and MPU6050 as a slave device
 * Slave address of the MPU6050 sensor is 0x68
 *
 * @param void
 * @return void
 */
void MLX90640_I2CInit()
{
	// Initialize the I2C bus
	ESP_ERROR_CHECK(i2c_new_master_bus(&master_bus_config, &master_bus_handle));
	ESP_ERROR_CHECK(i2c_master_bus_add_device(master_bus_handle, &master_device_config, &master_dev_handle));
	// Probe the slave device
	ESP_ERROR_CHECK(i2c_master_probe(master_bus_handle, MLX90640_SLAVE_ADR, I2C_TIMEOUT_MS));
}

/**
 * @brief Power on reset operation
 * 
 * If you check the datasheet the POR is mentioned. Should be writing 0x06 to 0x00, which resets all the register settings
 * 
 * @return int 
 */
int MLX90640_I2CGeneralReset()
{
	uint8_t write_buffer[2] = {0x00, 0x06};
	int ack = i2c_master_transmit(master_dev_handle, write_buffer, 2, I2C_TIMEOUT_MS);
	return ack;
}

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
	if (nMemAddressRead * 2 > 1664)
	{
		ESP_LOGI(TAG, "Error: Too many bytes to read. Max 832 words allowed (1664 bytes)");
		return -1;
	}

	uint8_t *write_buffer = (uint8_t *)malloc(2);
	if (write_buffer == NULL)
	{
		ESP_LOGI(TAG, "Error: Failed to allocate memory for write_buffer");
		return -1;
	}

	uint8_t *read_buffer = (uint8_t *)malloc(nMemAddressRead * 2);
	if (read_buffer == NULL)
	{
		ESP_LOGI(TAG, "Error: Failed to allocate memory for read_buffer");
		free(write_buffer);
		return -1;
	}

	esp_err_t err;

	// Prepare write buffer
	write_buffer[0] = startAddress >> 8;
	write_buffer[1] = startAddress & 0x00FF;

	err = i2c_master_transmit_receive(master_dev_handle, write_buffer, 2, read_buffer, nMemAddressRead * 2, I2C_TIMEOUT_MS);
	if (err != ESP_OK)
	{
		free(write_buffer);
		free(read_buffer);
		return -1;
	}

	for (int i = 0; i < nMemAddressRead; i++)
	{
		data[i] = read_buffer[i * 2] << 8 | read_buffer[i * 2 + 1];
	}

	free(write_buffer);
	free(read_buffer);
	return 0;
}

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
	uint8_t write_buffer[4];

	write_buffer[0] = writeAddress >> 8;
	write_buffer[1] = writeAddress & 0x00FF;
	write_buffer[2] = data >> 8;
	write_buffer[3] = data & 0x00FF;

	esp_err_t err = i2c_master_transmit(master_dev_handle, write_buffer, 4, I2C_TIMEOUT_MS);
	if (err != ESP_OK)
	{
		ESP_LOGI(TAG, "Error: %s", esp_err_to_name(err));
		return (int)err;
	}

	return 0;
}

void MLX90640_I2CFreqSet(int freq)
{
	// Not implemented
}