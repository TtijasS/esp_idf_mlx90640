#ifndef CONSTANTS_H
#define CONSTANTS_H

static const char *TAG = "I2C Example";

#define I2C_SCL_IO CONFIG_I2C_MASTER_SCL // GPIO number used for I2C master clock
#define I2C_SDA_IO CONFIG_I2C_MASTER_SDA // GPIO number used for I2C master data
#define I2C_PORT_NUM I2C_NUM_0			 // I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip
#define I2C_FREQ_HZ 100000				 // I2C master clock frequency
#define I2C_TIMEOUT_MS 1000				 // I2C timeout in milliseconds

// Setup I2C buffer sizes
#define I2C_READ_BUFF_SIZE 768 // I2C max read buffer size
#define I2C_WRITE_BUFF_SIZE 4	// i2c max write buffer size

// MLX REGISTERS FOR COMMUNICATION
#define MLC_SLAVE_ADR 0x33 // MPU6050 slave address

// MLX90640 SETTINGS REGISTERS
#define MLX_RAM_REG 0x0400		  // RAM register for reading the measurements from the sensor
#define MLX_STAT_REG 0x8000		  // Status register
#define MLX_CTLR_REG_1 0x800D	  // Control register 1
#define MLX_CTLR_REG_2 0x800E	  // Control register 2
#define MLX_I2C_CONFIG_REG 0x800F // I2C configuration register

// MLX90640 REGISTERS
#define MLX_RAM_REG 0x0400		  // RAM register for reading the measurements from the sensor
#define MLX_STAT_REG 0x8000		  // Status register
#define MLX_CTLR_REG_1 0x800D	  // Control register 1
#define MLX_CTLR_REG_2 0x800E	  // Control register 2
#define MLX_I2C_CONFIG_REG 0x800F // I2C configuration register

// EEPROM Addresses for Calibration Parameters (each 16-bit word)
#define MLX_EEPROM_DUMP_REG 0x2410		  // EEPROM consts start register (for burst reading)
#define MLX_EEPROM_PTAT_25_REG 0x2430	  // Gain parameter
#define MLX_EEPROM_PTAT_25_REG 0x2431	  // PTAT parameter
#define MLX_EEPROM_VDD_25_REG 0x2432	  // VDD parameter
#define MLX_EEPROM_KPTAT_REG 0x2436		  // K_PTAT parameter
#define MLX_EEPROM_TGC_REG 0x2438		  // Temperature Gradient Coefficient (TGC) parameter
#define MLX_EEPROM_SENS_KV_REG 0x243A	  // Sensitivity parameter Kv
#define MLX_EEPROM_SENS_KTA_REG 0x243C	  // Sensitivity parameter Kta
#define MLX_EEPROM_KS_TO1_REG 0x2440	  // KsTo parameter 1
#define MLX_EEPROM_KS_TO2_REG 0x2442	  // KsTo parameter 2
#define MLX_EEPROM_KS_TO3_REG 0x2444	  // KsTo parameter 3
#define MLX_EEPROM_KS_TO4_REG 0x2446	  // KsTo parameter 4
#define MLX_EEPROM_CP1_REG 0x2448		  // Compensation pixel parameter CP1
#define MLX_EEPROM_CP2_REG 0x244A		  // Compensation pixel parameter CP2
#define MLX_EEPROM_PXL_OFFSETS_REG 0x244C // Offset parameters for each pixel

// MLX90640 REG SETTINGS

#endif // CONSTANTS_H
