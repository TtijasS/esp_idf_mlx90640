#ifndef APP_TASKS_H
#define APP_TASKS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "constants.h"
#include "mlx90640_api.h"
#include "mlx90640_i2c_driver.h"
#include "custom_mlx_functions.h"
#include "uart_isr_handler.h"

extern SemaphoreHandle_t semphr_request_image;

extern QueueHandle_t queue_uart_isr_event_queue; // UART ISR queue
extern QueueHandle_t queue_enqueued_msg_processing;

typedef struct TaskQueueMessage_type
{
	size_t msg_size;
	uint8_t *msg_ptr;
	
}TaskQueueMessage_type;


extern float *subpage_0;
extern float *subpage_1;

// MLX tasks
void task_initialization(void *params);
void task_mlx_get_subpages(void *params);
void task_mlx_merge_subpages(void *params);
void task_mlx_uart_frame_data(void *params);

// UART ISR MONITORING
void task_uart_isr_monitoring(void *);
void task_queue_msg_handler(void *);

#endif // APP_TASKS_H