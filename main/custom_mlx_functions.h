#ifndef CUSTOM_MLX_FUNCTIONS_H
#define CUSTOM_MLX_FUNCTONS_H

#include "stdlib.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "constants.h"
#include "mlx90640_api.h"
#include "uart_isr_handler.h"


extern paramsMLX90640 mlx90640_params;

void mlx_delay_after_por();
int mlx_read_extract_eeprom();
int mlx_get_subpage_temps(float *, float , int8_t , uint8_t , TickType_t *);
int mlx_read_full_picture(float *, float *, float , int8_t , TickType_t *);
int mlx_merge_subpages(float *, float *);


#endif // CUSTOM_MLX_FUNCTIONS_H