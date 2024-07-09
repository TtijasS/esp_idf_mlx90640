#include "custom_uart.h"

const uart_port_t uart_num = UART_NUM_0;
const int uart_buffer_size = (772 * sizeof(float)*4);

uart_config_t uart_config = {
	.baud_rate = UART_BAUD_RATE, // You can change this to your required baud rate
	.data_bits = UART_DATA_8_BITS,
	.parity = UART_PARITY_DISABLE,
	.stop_bits = UART_STOP_BITS_1,
	.flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

void uart_init()
{
	ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

	// Set UART pins (TX: GPIO43, RX: GPIO44, RTS: unused, CTS: unused)
	ESP_ERROR_CHECK(uart_set_pin(uart_num, 43, 44, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

	// Install UART driver using an event queue here
	ESP_ERROR_CHECK(uart_driver_install(uart_num, uart_buffer_size, uart_buffer_size, 10, NULL, 0));
}

void send_float_over_uart(float value)
{
    uint8_t data[sizeof(float)];
    memcpy(data, &value, sizeof(float));
    uart_write_bytes(uart_num, (const char *)data, sizeof(float));
}

void send_frame_over_uart(float *frame_data)
{
    uart_write_bytes(uart_num, "\xfe\xfe\xfe\xfe", 4);

    // Size of each chunk
    const size_t chunk_size = 384; // Adjust based on your UART buffer capacity
    const size_t total_floats = 768;
    uint8_t buffer[chunk_size * sizeof(float)];

    for (size_t i = 0; i < total_floats; i += chunk_size)
    {
        size_t current_chunk_size = (total_floats - i) < chunk_size ? (total_floats - i) : chunk_size;
        memcpy(buffer, &frame_data[i], current_chunk_size * sizeof(float));
        uart_write_bytes(uart_num, (const char *)buffer, current_chunk_size * sizeof(float));
    }

    return;
}