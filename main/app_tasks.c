#include "app_tasks.h"

TaskHandle_t handl_get_subpages;
TaskHandle_t handl_merge_subpages;
TaskHandle_t handl_uart_frame_data;
TaskHandle_t handl_mlx_init;

SemaphoreHandle_t semphr_request_image;

QueueHandle_t queue_uart_isr_event_queue; // UART ISR queue
QueueHandle_t queue_enqueued_msg_processing;

float *subpage_0;
float *subpage_1;

void task_initialization(void *params)
{
	const char *TAG = "TSK INIT";
	int error_code = 0;

	subpage_0 = (float *)calloc(MLX_FRAME_SIZE, sizeof(float));
	subpage_1 = (float *)calloc(MLX_FRAME_SIZE, sizeof(float));
	if (subpage_0 == NULL || subpage_1 == NULL)
	{
		ESP_LOGE(TAG, "Failed to allocate memory for subpage 0 and or 1");
		vTaskDelete(NULL);
	}

	// Create semaphore binaries
	semphr_request_image = xSemaphoreCreateBinary();

	// Create queues
	queue_uart_isr_event_queue = xQueueCreate(8, sizeof(TaskQueueMessage_type));
	queue_enqueued_msg_processing = xQueueCreate(4, sizeof(TaskQueueMessage_type));

	// Init MLX I2C
	if ((error_code = MLX90640_I2CInit()) != 0)
	{
		ESP_LOGE(TAG, "Failed to init i2c. Error: %d", error_code);
		vTaskDelete(NULL);
	}
	// Set camera refresh rate
	if ((error_code = MLX90640_SetRefreshRate(MLX90640_SLAVE_ADR, MLX_REFRESH_RATE)) != 0)
	{
		ESP_LOGE(TAG, "Failed to set mlx frame refresh. Error: %d", error_code);
		vTaskDelete(NULL);
	}
	// General reset MLX
	if ((error_code = MLX90640_I2CGeneralReset()) != 0)
	{
		ESP_LOGE(TAG, "Failed to reset the sensor. Error: %d", error_code);
		vTaskDelete(NULL);
	}
	// Initial MLX delay after power-on reset
	mlx_delay_after_por();
	// Read frame
	if (mlx_read_extract_eeprom(&mlx90640_params) != 0)
	{
		ESP_LOGE(TAG, "Failed to read and extract EEPROM data");
		vTaskDelete(NULL);
	}

	if (xTaskCreatePinnedToCore(task_mlx_get_subpages, "MLX get subpage task", TASK_GET_SUBPAGES_STACK_SIZE, NULL, 10, &handl_get_subpages, tskNO_AFFINITY) != pdPASS)
	{
		ESP_LOGE(TAG, "Failed to create mlx get subpage task");
		vTaskDelete(NULL);
	}
	if (xTaskCreatePinnedToCore(task_mlx_merge_subpages, "MLX merge subpages task", TASK_MERGE_SUBPAGES_STACK_SIZE, NULL, 10, &handl_merge_subpages, tskNO_AFFINITY) != pdPASS)
	{
		ESP_LOGE(TAG, "Failed to create mlx mlx merge subpages task");
		vTaskDelete(NULL);
	}
	if (xTaskCreatePinnedToCore(task_mlx_uart_frame_data, "MLX merge subpages task", TASK_UART_FRAME_DATA_STACK_SIZE, NULL, 10, &handl_uart_frame_data, tskNO_AFFINITY) != pdPASS)
	{
		ESP_LOGE(TAG, "Failed to create mlx mlx merge subpages task");
		vTaskDelete(NULL);
	}

	// Init UART with ISR queue
	if ((error_code = myuart_init_with_isr_queue(&uart_config, UART_NUM, UART_TXD, UART_RXD, UART_TX_BUFF_SIZE, UART_RX_BUFF_SIZE, &queue_uart_isr_event_queue, UART_EVENT_QUEUE_SIZE, 0)) != 0)
	{
		ESP_LOGE(TAG, "Failed to init uart with isr queue. Error code %d", error_code);
		vTaskDelete(NULL);
	}

	// Create UART ISR tasks
	if (xTaskCreatePinnedToCore(&task_uart_isr_monitoring, "UART ISR monitoring task", TASK_ISRUART_STACK_SIZE, NULL, 18, NULL, 0) != pdPASS)
	{
		ESP_LOGE(TAG, "Failed to create uart isr monitoring task");
		vTaskDelete(NULL);
	}
	if (xTaskCreatePinnedToCore(&task_queue_msg_handler, "Receive queue msg task", TASK_MSG_Q_STACK_SIZE, NULL, 10, NULL, 1) != pdPASS)
	{
		ESP_LOGE(TAG, "Failed to create receive queue msg task");
		vTaskDelete(NULL);
	}
	// Enable pattern detection and reset the pattern queue
	if ((error_code = uart_enable_pattern_det_baud_intr(UART_NUM, '+', UART_PATTERN_SIZE, 8, 0, 0)) != 0)
	{
		ESP_LOGE(TAG, "Failed to enable pattern detection baud interrupt");
		vTaskDelete(NULL);
	}
	if ((error_code = uart_pattern_queue_reset(UART_NUM, UART_PAT_QUEUE_SIZE)) != 0)
	{
		ESP_LOGE(TAG, "Failed to reset uart pattern queue");
		vTaskDelete(NULL);
	}

	// Initial semaphore give
	xSemaphoreGive(semphr_request_image);

	if (DEBUG_STACKS == 1)
	{
		UBaseType_t stack_hwm = uxTaskGetStackHighWaterMark(NULL);
		ESP_LOGD(TAG, "Free stack size: %u B", stack_hwm);
		ESP_LOGD(TAG, "Stack in use: %u of %u B", (TASK_INIT_STACK_SIZE - stack_hwm), TASK_INIT_STACK_SIZE);
	}

	vTaskDelete(NULL);
}

/**
 * @brief
 *
 * @param params
 */
void task_mlx_get_subpages(void *params)
{
	const char *TAG = "TSK GET SUBPAGES";

	TickType_t last_wake_time = 0;
	uint8_t failed_attempts = 0;
	int error_code = 0;
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		// Zero write subpages
		memset(subpage_0, 0, MLX_FRAME_SIZE * sizeof(float));
		memset(subpage_1, 0, MLX_FRAME_SIZE * sizeof(float));

		// Synchronize the frame
		if (MLX90640_SynchFrame(MLX90640_SLAVE_ADR) != 0)
		{
			ESP_LOGW(TAG, "Failed syncing subpages. Error: %d", error_code);
			xSemaphoreGive(semphr_request_image);
			continue;
		}

		while ((failed_attempts < 2) &&
			   (error_code = mlx_read_full_picture(subpage_0, subpage_1, .97, -8, &last_wake_time)) != 0)
		{
			// If reading picture failed give it another try
			failed_attempts++;
			MLX90640_SynchFrame(MLX90640_SLAVE_ADR);
		}

		if (error_code != 0)
		{
			ESP_LOGW(TAG, "Failed reading subpages. Error: %d", error_code);
			xSemaphoreGive(semphr_request_image);
			continue;
		}

		if (DEBUG_STACKS == 1)
		{
			UBaseType_t stack_hwm = uxTaskGetStackHighWaterMark(NULL);
			ESP_LOGD(TAG, "Free stack size: %u B", stack_hwm);
			ESP_LOGD(TAG, "Stack in use: %u of %u B", (TASK_GET_SUBPAGES_STACK_SIZE - stack_hwm), TASK_GET_SUBPAGES_STACK_SIZE);
		}

		failed_attempts = 0;
		error_code = 0;
		xTaskNotifyGive(handl_merge_subpages);
	}
}

void task_mlx_merge_subpages(void *params)
{
	const char *TAG = "TSK MERGE SUBPAGES";
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (mlx_merge_subpages(subpage_0, subpage_1) != 0)
		{
			ESP_LOGW(TAG, "Failed to merge subpages");
			uart_write_bytes(UART_NUM, "Error reading subpages", strlen("Error reading subpages"));
			xSemaphoreGive(semphr_request_image);
			continue;
		}

		if (DEBUG_STACKS == 1)
		{
			UBaseType_t stack_hwm = uxTaskGetStackHighWaterMark(NULL);
			ESP_LOGD(TAG, "Free stack size: %u B", stack_hwm);
			ESP_LOGD(TAG, "Stack in use: %u of %u B", (TASK_MERGE_SUBPAGES_STACK_SIZE - stack_hwm), TASK_MERGE_SUBPAGES_STACK_SIZE);
		}

		xTaskNotifyGive(handl_uart_frame_data);
	}
}

void task_mlx_uart_frame_data(void *params)
{
	const char *TAG = "TSK UART FRAME DATA";
	size_t buffer_size = MLX_FRAME_SIZE * sizeof(float);
	uint8_t *buffer = (uint8_t *)malloc(buffer_size);
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		// copy new frame bytes to uint8_t buffer
		memcpy(buffer, subpage_0, MLX_FRAME_SIZE * sizeof(float));
		// Start flag, data, stop flag
		uart_write_bytes(UART_NUM, "\xff\xff\xff\xff\xfa", 5);
		uart_write_bytes(UART_NUM, buffer, buffer_size);
		uart_write_bytes(UART_NUM, "\xfa\xff\xff\xff\xff", 5);

		if (DEBUG_STACKS == 1)
		{
			UBaseType_t stack_hwm = uxTaskGetStackHighWaterMark(NULL);
			ESP_LOGD(TAG, "Free stack size: %u B", stack_hwm);
			ESP_LOGD(TAG, "Stack in use: %u of %u B", (TASK_UART_FRAME_DATA_STACK_SIZE - stack_hwm), TASK_UART_FRAME_DATA_STACK_SIZE);
		}

		xSemaphoreGive(semphr_request_image);
	}
}

// ---------- UART ISR TASKS ----------

/**
 * @brief Uart ISR event monitoring
 *
 * Catch encapsulated uart messages and send them to the message processing task
 *
 * @param params
 */
void task_uart_isr_monitoring(void *params)
{
	const char *TAG = "UART ISR TASK";

	uart_event_t uart_event;
	int error_flag = 0;
	int encapsulation_counter = 0;

	while (1)
	{
		// Receive the entire uart_event_t structure from the queue
		if (xQueueReceive(queue_uart_isr_event_queue, (void *)&uart_event, portMAX_DELAY) == pdTRUE)
		{
			switch (uart_event.type)
			{
			/**
			 * @brief Pattern detection case
			 *
			 * Multiple identical characters of specific type detected in a row.
			 * Number of chars is defined with macro UART_PATTERN_SIZE
			 */
			case UART_PATTERN_DET:
				int pattern_index = uart_pattern_pop_pos(UART_NUM);
				if (pattern_index == -1)
				{
					ESP_LOGW(TAG, "Pattern index -1");
					break;
				}
				if ((error_flag = myuart_encapsulation_handler(UART_NUM, &encapsulation_counter, &pattern_index)) < 0)
				{
					ESP_LOGE(TAG, "Uart encapsulation handler error %d", error_flag);
				}
				break;

			default:
				break;
			}
		}

		if (DEBUG_STACKS == 1)
		{
			UBaseType_t stack_hwm = uxTaskGetStackHighWaterMark(NULL);
			ESP_LOGD(TAG, "Free stack size: %u B", stack_hwm);
			ESP_LOGD(TAG, "Stack in use: %u of %u B", (TASK_ISRUART_STACK_SIZE - stack_hwm), TASK_ISRUART_STACK_SIZE);
		}
	}
	ESP_LOGW(TAG, "KILLING THE UART ISR TASK");
	vTaskDelete(NULL);
}

/**
 * @brief Encapsulated message handler
 *
 * Process encapsulated messages received from the uart isr monitoring task.
 * Trigger MLX90640 measuring with character sequence.
 *
 * @param params
 */
void task_queue_msg_handler(void *params)
{
	const char *TAG = "TSK QUEUE MSG HANDL";

	TaskQueueMessage_type enqueued_message;

	// START SAMPLING
	const char *MLX_START = "MLX START";
	// RESPONSES
	const char *MLX_BUSY = "MLX BUSY";
	const char *MLX_OK = "MLX OK";
	// const char *MLX_FAIL = "MLX FAIL";
	// GENERIC
	const char *WHOAMI = "WHOAMI";
	const char *DEVID = "MLX90640";

	while (1)
	{
		if (xQueueReceive(queue_enqueued_msg_processing, &enqueued_message, portMAX_DELAY))
		{
			if (enqueued_message.msg_ptr != NULL)
			{
				// WHOAMI
				if (memcmp(enqueued_message.msg_ptr, WHOAMI, (strlen(WHOAMI))) == 0)
				{
					uart_write_bytes(UART_NUM, DEVID, strlen(DEVID));
				}
				// A START
				else if (memcmp(enqueued_message.msg_ptr, MLX_START, (strlen(MLX_START))) == 0)
				{
					if (xSemaphoreTake(semphr_request_image, pdMS_TO_TICKS(10)) == pdTRUE)
					{
						uart_write_bytes(UART_NUM, MLX_OK, strlen(MLX_OK));
						xTaskNotifyGive(handl_get_subpages);
					}
					else
					{
						uart_write_bytes(UART_NUM, MLX_BUSY, strlen(MLX_BUSY));
					}
				}
				else
				{
					uart_write_bytes(UART_NUM, "??", strlen("??"));
					uart_write_bytes(UART_NUM, enqueued_message.msg_ptr, enqueued_message.msg_size);
				}
				free(enqueued_message.msg_ptr);
			}
			else
			{
				uart_write_bytes(UART_NUM, "Null pointer passed", strlen("Null pointer passed"));
				ESP_LOGW(TAG, "Null pointer passed");
			}
		}

		if (DEBUG_STACKS == 1)
		{
			UBaseType_t stack_hwm = uxTaskGetStackHighWaterMark(NULL);
			ESP_LOGD(TAG, "Free stack size: %u B", stack_hwm);
			ESP_LOGD(TAG, "Stack in use: %u of %u B", (TASK_MSG_Q_STACK_SIZE - stack_hwm), TASK_MSG_Q_STACK_SIZE);
		}
	}
}