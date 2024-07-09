#ifndef CONSTANTS_H
#define CONSTANTS_H
// ----------------- Constants -----------------
static const char *TAG = "MLX";

#define I2C_SCL_IO CONFIG_I2C_MASTER_SCL // GPIO number used for I2C master clock
#define I2C_SDA_IO CONFIG_I2C_MASTER_SDA // GPIO number used for I2C master data
#define I2C_PORT_NUM I2C_NUM_0           // I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip
#define I2C_FREQ_HZ 100000               // I2C master clock frequency
#define UART_BAUD_RATE 3000000            // UART baud rate
#define I2C_TIMEOUT_MS 1000              // I2C timeout in milliseconds
#define MLX90640_SLAVE_ADR 0x33


/*0 0 0 IR refresh rate = 0.5Hz
0 0 1 IR refresh rate = 1Hz
0 1 0 IR refresh rate = 2Hz (default)
0 1 1 IR refresh rate = 4Hz
1 0 0 IR refresh rate = 8Hz
1 0 1 IR refresh rate = 16Hz
1 1 0 IR refresh rate = 32Hz
1 1 1 IR refresh rate = 64Hz*/
#define MLX_REFRESH_0_5HZ 0x00
#define MLX_REFRESH_1HZ 0x01
#define MLX_REFRESH_2HZ 0x02
#define MLX_REFRESH_4HZ 0x03
#define MLX_REFRESH_8HZ 0x04
#define MLX_REFRESH_16HZ 0x05
#define MLX_REFRESH_32HZ 0x06
#define MLX_REFRESH_64HZ 0x07

#define MLX_0_5_HZ_MILLIS 2000
#define MLX_1_HZ_MILLIS 1000
#define MLX_2_HZ_MILLIS 500
#define MLX_4_HZ_MILLIS 250
#define MLX_8_HZ_MILLIS 125
#define MLX_16_HZ_MILLIS 62
#define MLX_32_HZ_MILLIS 31
#define MLX_64_HZ_MILLIS 15

// ############################# REFRESH CONFIGURATION #############################
// SETUP REFRESH RATE AND DELAY MILLIS
// you must set both to the same value
#define MLX_REFRESH_RATE MLX_REFRESH_4HZ
#define MLX_REFRESH_MILLIS MLX_4_HZ_MILLIS
// #################################################################################
extern const int DELAY_BETWEEN_SUBPAGES;

#endif // CONSTANTS_H