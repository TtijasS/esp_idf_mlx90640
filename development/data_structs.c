#include "mlx90640_i2c_driver.h"

// Initialize the MLX90640 sensor data structure
mlx_data_type mpu_data = {
	.gain = 0,
	.vdd_25 = 0,
	.ptat_25 = 0,
	.kptat = 0,
	.tgc = 0,
	.ks_to = {0, 0, 0, 0}, // Assuming four values for KsTo parameters
	.cp_params = {0, 0},   // Assuming two compensation parameters
	.sens_params = {0, 0}, // Assuming two sensitivity parameters (Kv, Kta)
	.pix_offsets = {0},	   // 768 pixel offsets
};

// Initialize the I2C buffer data structure
i2c_buffer_type i2c_buffer = {
	.read_buffer = {0},
	.write_buffer = {0},
};