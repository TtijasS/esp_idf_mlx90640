#ifndef CONSTANTS_H
#define CONSTANTS_H

static const char *TAG = "I2C Example";

#define I2C_SCL_IO CONFIG_I2C_MASTER_SCL // GPIO number used for I2C master clock
#define I2C_SDA_IO CONFIG_I2C_MASTER_SDA // GPIO number used for I2C master data
#define I2C_PORT_NUM I2C_NUM_0			 // I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip
#define I2C_FREQ_HZ 100000				 // I2C master clock frequency
#define I2C_TIMEOUT_MS 1000				 // I2C timeout in milliseconds

// Setup I2C buffer sizes
#define I2C_READ_BUFF_SIZE 385 // I2C max read buffer size
#define I2C_WRITE_BUFF_SIZE 4 // i2c max write buffer size

// MLX REGISTERS FOR COMMUNICATION
#define MLC_SLAVE_ADR 0x33	  // MPU6050 slave address

// MLX90640 REGISTERS
#define MLX_STAT_REG 0X8000
#define MLX_CTLR_REG_1 0X800D
#define MLX_CTLR_REG_2 0X800E
#define MLX_I2C_CONFIG_REG 0X800F

// MLX90640 REG SETTINGS

#endif // CONSTANTS_H
