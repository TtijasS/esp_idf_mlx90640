/**
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef _MLX90640_I2C_Driver_H_
#define _MLX90640_I2C_Driver_H_

#include <stdint.h>
#include <esp_log.h>
#include <driver/i2c_master.h>
#include "constants.h"
#include "mlx90640_i2c_driver.h"

// Extern declarations for global configurations and handles
extern const i2c_master_bus_config_t i2c_master_bus_config;
extern const i2c_device_config_t i2c_master_device_config;
extern i2c_master_bus_handle_t master_bus_handle;
extern i2c_master_dev_handle_t master_dev_handle;
extern int MLX90640_I2CInit(void);
extern int MLX90640_I2CGeneralReset(void);
extern int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data);
extern int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data);
extern void MLX90640_I2CFreqSet(int freq);
#endif