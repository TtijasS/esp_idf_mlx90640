#ifndef CONSTANTS_H
#define CONSTANTS_H
// ----------------- Constants -----------------
extern const int DELAY_BETWEEN_SUBPAGES;

#define I2C_SCL_IO CONFIG_I2C_MASTER_SCL // GPIO number used for I2C master clock
#define I2C_SDA_IO CONFIG_I2C_MASTER_SDA // GPIO number used for I2C master data
#define I2C_PORT_NUM I2C_NUM_0			 // I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip
#define I2C_FREQ_HZ 400000				 // I2C master clock frequency
#define I2C_TIMEOUT_MS 1000				 // I2C timeout in milliseconds
#define MLX90640_SLAVE_ADR 0x33
#define MLX_FRAME_SIZE 768

// Stack sizes
#define TASK_INIT_STACK_SIZE (1024*5)
#define TASK_GET_SUBPAGES_STACK_SIZE (1024*4)
#define TASK_UART_FRAME_DATA_STACK_SIZE (1024*2)
#define TASK_MERGE_SUBPAGES_STACK_SIZE (1024*2)
#define TASK_ISRUART_STACK_SIZE (1024*2)
#define TASK_MSG_Q_STACK_SIZE (1024*2)
#define DEBUG_STACKS 0

// ############################# REFRESH CONFIGURATION #############################
// --------- UNCOMMENT ONE OF THE FOLLOWING LINES TO SET THE REFRESH RATE ---------
// #define MLX_REFRESH_1_HZ 0x01
// #define MLX_REFRESH_2_HZ 0x02
// #define MLX_REFRESH_4_HZ 0x03
#define MLX_REFRESH_8_HZ 0x04
// #define MLX_REFRESH_16_HZ 0x05
// #define MLX_REFRESH_32_HZ 0x06
// #define MLX_REFRESH_64_HZ 0x07
// #################################################################################

// DONT COMMENT OR CHANGE THESE VALUES
#define MLX_REFRESH_0_5_HZ 0x00 // default refresh rate mask.

#define MLX_0_5_HZ_MILLIS 2000
#define MLX_1_HZ_MILLIS 1000
#define MLX_2_HZ_MILLIS 500
#define MLX_4_HZ_MILLIS 250
#define MLX_8_HZ_MILLIS 125
#define MLX_16_HZ_MILLIS 62
#define MLX_32_HZ_MILLIS 31
#define MLX_64_HZ_MILLIS 15

#if defined(MLX_REFRESH_1_HZ)
#define MLX_REFRESH_RATE MLX_REFRESH_1_HZ
#define MLX_REFRESH_MILLIS MLX_1_HZ_MILLIS
#elif defined(MLX_REFRESH_2_HZ)
#define MLX_REFRESH_RATE MLX_REFRESH_2_HZ
#define MLX_REFRESH_MILLIS MLX_2_HZ_MILLIS
#elif defined(MLX_REFRESH_4_HZ)
#define MLX_REFRESH_RATE MLX_REFRESH_4_HZ
#define MLX_REFRESH_MILLIS MLX_4_HZ_MILLIS
#elif defined(MLX_REFRESH_8_HZ)
#define MLX_REFRESH_RATE MLX_REFRESH_8_HZ
#define MLX_REFRESH_MILLIS MLX_8_HZ_MILLIS
#elif defined(MLX_REFRESH_16_HZ)
#define MLX_REFRESH_RATE MLX_REFRESH_16_HZ
#define MLX_REFRESH_MILLIS MLX_16_HZ_MILLIS
#elif defined(MLX_REFRESH_32_HZ)
#define MLX_REFRESH_RATE MLX_REFRESH_32_HZ
#define MLX_REFRESH_MILLIS MLX_32_HZ_MILLIS
#elif defined(MLX_REFRESH_64_HZ)
#define MLX_REFRESH_RATE MLX_REFRESH_64_HZ
#define MLX_REFRESH_MILLIS MLX_64_HZ_MILLIS
#else
// default value is 0.5Hz
#define MLX_REFRESH_RATE MLX_REFRESH_0_5_HZ
#define MLX_REFRESH_MILLIS MLX_0_5_HZ_MILLIS
#endif

#endif // CONSTANTS_H