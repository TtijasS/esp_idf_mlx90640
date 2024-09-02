#include "mlx90640_i2c_driver.h"

// Define the I2C master bus configuration
const i2c_master_bus_config_t i2c_master_bus_config = {
	.i2c_port = I2C_NUM_0,
	.sda_io_num = I2C_SDA_IO,
	.scl_io_num = I2C_SCL_IO,
	.clk_source = I2C_CLK_SRC_DEFAULT,
	.glitch_ignore_cnt = 7,
	.flags.enable_internal_pullup = true};

// Define the I2C device configuration
const i2c_device_config_t i2c_master_device_config = {
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
int MLX90640_I2CInit()
{
	const char *TAG = "MLX90640_I2CInit";
	int error_code = 0;

	// Initialize the I2C bus

	if ((error_code = i2c_new_master_bus(&i2c_master_bus_config, &master_bus_handle)) != 0)
	{
		ESP_LOGE(TAG, "Failed to create new i2c master bus. Error %d", error_code);
		return -1;
	}
	if ((error_code = i2c_master_bus_add_device(master_bus_handle, &i2c_master_device_config, &master_dev_handle)) != 0)
	{
		ESP_LOGE(TAG, "Failed to add new i2c device to master bus. Error %d", error_code);
		return -2;
	}
	if ((error_code = i2c_master_probe(master_bus_handle, MLX90640_SLAVE_ADR, I2C_TIMEOUT_MS)) != 0)
	{
		ESP_LOGE(TAG, "Failed to master probe. Error %d", error_code);
		return -3;
	}
	return 0;
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
	int error_code = 0;
	const char *TAG = "MLX90640_I2cRead";
	if ((nMemAddressRead * 2) > (832 * 2))
	{
		ESP_LOGI(TAG, "Error: Too many bytes to read. Max 832 words allowed (1664 bytes)");
		return -1;
	}

	uint8_t write_buffer[2] = {0};

	uint8_t *read_buffer = (uint8_t *)malloc(nMemAddressRead * 2);
	if (read_buffer == NULL)
	{
		ESP_LOGI(TAG, "Error: Failed to allocate memory for read_buffer");
		return -2;
	}

	// Initialize read buffer to 0
	for (int i = 0; i < nMemAddressRead * 2; i++)
	{
		read_buffer[i] = 0;
	}

	// Prepare write buffer
	write_buffer[0] = startAddress >> 8;
	write_buffer[1] = startAddress & 0x00FF;

	if (i2c_master_transmit_receive(master_dev_handle, write_buffer, 2, read_buffer, nMemAddressRead * 2, I2C_TIMEOUT_MS) != ESP_OK)
	{
		free(read_buffer);
		return -3;
		
	}

	for (int i = 0; i < nMemAddressRead; i++)
	{
		data[i] = read_buffer[i * 2] << 8 | read_buffer[i * 2 + 1];
	}

	free(read_buffer);
	return 0;
}

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
	const char *TAG = "MLX90640_I2CWrite";
	uint8_t write_buffer[4];

	write_buffer[0] = writeAddress >> 8;
	write_buffer[1] = writeAddress & 0x00FF;
	write_buffer[2] = data >> 8;
	write_buffer[3] = data & 0x00FF;

	if (i2c_master_transmit(master_dev_handle, write_buffer, 4, I2C_TIMEOUT_MS) != ESP_OK)
	{
		ESP_LOGI(TAG, "Error i2c master transmit");
		return -1;
	}

	return 0;
}

void MLX90640_I2CFreqSet(int freq)
{
	// Not implemented
}