#ifndef APP_TASKS_H
#define APP_TASKS_H

#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "custom_uart.h"
#include "constants.h"
#include "mlx90640_api.h"
#include "mlx90640_i2c_driver.h"
#include "custom_mlx_functions.h"

extern TaskHandle_t task_read_frame_handle;
extern TaskHandle_t task_uart_send_frame_handle;
extern TaskHandle_t task_synchronize_handle;

extern TickType_t last_wake_time;
extern TickType_t deltatime;
extern int deltatime_diff;
extern float *subpage_0;
extern float *subpage_1;
extern float *whole_frame;

void task_initialization(void *);
void task_read_frame(void *);
void task_uart_send_frame(void *);
void task_apply_synchronization_delay(void *pvParameters);




#endif // APP_TASKS_H