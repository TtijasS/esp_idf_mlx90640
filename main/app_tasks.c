#include "app_tasks.h"

TaskHandle_t task_read_frame_handle = NULL;
TaskHandle_t task_uart_send_frame_handle = NULL;
TaskHandle_t task_synchronize_handle = NULL;

TickType_t last_wake_time = 0;
TickType_t deltatime = 0;
int deltatime_diff = 0;
float *subpage_0 = NULL;
float *subpage_1 = NULL;
float *whole_frame = NULL;

void task_initialization(void *pvParameters)
{
	uart_init();
	MLX90640_I2CInit();
	MLX90640_SetRefreshRate(MLX90640_SLAVE_ADR, MLX_REFRESH_RATE);

	int refresh_setting = MLX90640_GetRefreshRate(MLX90640_SLAVE_ADR);
	ESP_LOGI(TAG, "Refresh rate set to %d Hz", refresh_setting);

	if (MLX90640_I2CGeneralReset() != 0)
	{
		ESP_LOGE(TAG, "Failed to reset the sensor");
		return;
	}

	// Delay after power on reset
	mlx_delay_after_por();

	// Read and extract EEPROM calibration data into mlx90640_params
	if (mlx_read_extract_eeprom(&mlx90640_params) != 0)
	{
		ESP_LOGE(TAG, "Failed to read and extract EEPROM data");
		return;
	}

	// Allocate memory for frame data
	subpage_0 = (float *)malloc(768 * sizeof(float));
	subpage_1 = (float *)malloc(768 * sizeof(float));
	whole_frame = (float *)malloc(768 * sizeof(float));
	if ((subpage_0 == NULL) | (subpage_1 == NULL))
	{
		ESP_LOGE(TAG, "Failed to allocate memory for frame data");
		return;
	}

	MLX90640_SynchFrame(MLX90640_SLAVE_ADR); // Sync the frame
	last_wake_time = xTaskGetTickCount();

	xTaskNotifyGive(task_read_frame_handle); // start reading the frame

	vTaskDelete(NULL); // delete task after successfull initialization
}

void task_read_frame(void *pvParameters)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		for (int i = 0; i < 768; i++)
		{
			subpage_0[i] = 0;
			subpage_1[i] = 0;
		}

		if (mlx_read_full_picture(subpage_0, subpage_1, .97, -8, &last_wake_time) != 0)
		{
			ESP_LOGE(TAG, "Failed to read the full picture");
			MLX90640_SynchFrame(MLX90640_SLAVE_ADR);
			continue;
		}

		xTaskNotifyGive(task_uart_send_frame_handle);
		xTaskNotifyGive(task_synchronize_handle);
	}
}

void task_merge_subpages(void *pvParameters)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		mlx_merge_subpages(subpage_0, subpage_1);
	}
}

void task_uart_send_frame(void *pvParameters)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		send_frame_over_uart(subpage_0);
		ulTaskNotifyTake
	}
}

void task_apply_synchronization_delay(void *pvParameters)
{
	while (1)
	{
		deltatime = pdTICKS_TO_MS(xTaskGetTickCount() - last_wake_time);
        deltatime_diff = DELAY_BETWEEN_SUBPAGES - deltatime;
        if (deltatime_diff > 0)
        {
            xTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(deltatime_diff));
            // xTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MLX_REFRESH_MILLIS));
        }
        else
        {
            ESP_LOGI(TAG, "deltatime_diff: %d", deltatime_diff);

        }
	}
}
