#ifndef MLX90640_H
#define MLX90640_H

#include <driver/i2c_master.h>
#include <freertos/FreeRTOS.h>
#include "constants.h"
#include "my_i2c_config.h"
#include "data_structs.h"
#include <esp_log.h>

// Functions prototypes
bool mlx_initial_setup(i2c_buffer_type *);
bool mlx_transmit_receive(i2c_buffer_type *, uint8_t, uint16_t);
bool mlx_transmit(i2c_buffer_type *, uint8_t);
bool mlx_configure_register(i2c_buffer_type *, uint16_t, uint16_t, bool);
bool mlx_read_register(i2c_buffer_type *, uint16_t, uint16_t *);
bool mlx_read_ram_data(i2c_buffer_type *);
bool mlx_read_eeprom_dump(i2c_buffer_type *, uint16_t *eeprom_dump);

#endif // MLX90640_H