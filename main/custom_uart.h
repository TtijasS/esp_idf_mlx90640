#ifndef CUSTOM_UART_H
#define CUSTOM_UART_H
#include "esp_log.h"
#include <string.h> // Include string.h for memcpy
#include <stdint.h>
#include "driver/uart.h"
#include "constants.h"

extern const uart_port_t uart_num;
extern const int uart_buffer_size;
extern uart_config_t uart_config;

void uart_init();
void send_float_over_uart(float value);
void send_frame_over_uart(float *frame_data);

#endif