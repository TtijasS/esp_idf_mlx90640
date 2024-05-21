#ifndef DATA_STRUCTS_H
#define DATA_STRUCTS_H

#include <freertos/FreeRTOS.h>
#include "constants.h"

/**
 * @brief Data structure for storing the MLX90640 sensor calibration and other data
 *
 * The data structure stores the calibration data and pixel-specific data for the MLX90640 sensor.
 */
typedef struct mlx_data_type
{
    int16_t gain;             // Gain parameter (0x2430, 2 bytes)
    int16_t vdd_25;           // VDD parameter (0x2432, 2 bytes)
    int16_t ptat_25;          // PTAT parameter (0x2434, 2 bytes)
    int16_t kptat;            // K_PTAT parameter (0x2436, 2 bytes)
    int16_t tgc;              // Temperature Gradient Coefficient (TGC) parameter (0x2438, 2 bytes)
    int16_t sens_kv;          // Sensitivity parameter Kv (0x243A, 2 bytes)
    int16_t sens_kta;         // Sensitivity parameter Kta (0x243C, 2 bytes)
    int16_t ks_to_1;          // KsTo parameter 1 (0x2440, 2 bytes)
    int16_t ks_to_2;          // KsTo parameter 2 (0x2442, 2 bytes)
    int16_t ks_to_3;          // KsTo parameter 3 (0x2444, 2 bytes)
    int16_t ks_to_4;          // KsTo parameter 4 (0x2446, 2 bytes)
    int16_t cp_1;             // Compensation pixel parameter CP1 (0x2448, 2 bytes)
    int16_t cp_2;             // Compensation pixel parameter CP2 (0x244A, 2 bytes)
    int16_t pix_offsets[768]; // Offset parameters for each pixel (0x244C to 0x273F, 2 bytes each)
} mlx_data_type;

/**
 * @brief Data structure for storing the I2C read and write buffers
 */
typedef struct i2c_buffer_type
{
    uint8_t read_buffer[I2C_READ_BUFF_SIZE];   // Read buffer for I2C communication
    uint8_t write_buffer[I2C_WRITE_BUFF_SIZE]; // Write buffer for I2C communication
} i2c_buffer_type;

// Declare the variables as extern to allow access from other files
extern mlx_data_type mpu_data;
extern i2c_buffer_type i2c_buffer;

#endif // DATA_STRUCTS_H