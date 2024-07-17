#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include <string.h> // Include string.h for memcpy
#include <stdio.h>
#include "mlx90640_i2c_driver.h"
#include "mlx90640_api.h"
#include "constants.h"
#include "custom_uart.h"
#include "custom_mlx_functions.h"


// ----------------- Main -----------------
void app_main(void)
{
    uart_init();
    MLX90640_I2CInit();
    MLX90640_SetRefreshRate(MLX90640_SLAVE_ADR, MLX_REFRESH_RATE);
    int refresh_setting = MLX90640_GetRefreshRate(MLX90640_SLAVE_ADR);
    ESP_LOGI(TAG, "I2C refresh rate: %d Hz", refresh_setting);
    ESP_LOGI(TAG, "Baud rate rate: %d baud", UART_BAUD_RATE);
    ESP_ERROR_CHECK(uart_set_baudrate(UART_NUM_0, UART_BAUD_RATE));
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
    float *subpage_0 = (float *)malloc(768 * sizeof(float));
    float *subpage_1 = (float *)malloc(768 * sizeof(float));
    if ((subpage_0 == NULL) || (subpage_1 == NULL))
    {
        ESP_LOGE(TAG, "Failed to allocate memory for frame data");
        return;
    }
    TickType_t last_wake_time = 0;
    TickType_t deltatime = 0;
    int deltatime_diff = 0;
    MLX90640_SynchFrame(MLX90640_SLAVE_ADR);
    vTaskDelay(pdMS_TO_TICKS(MLX_REFRESH_MILLIS));
    while (1)
    {
        for (int i = 0; i < 768; i++)
        {
            subpage_0[i] = 0;
            subpage_1[i] = 0;
        }

        if (mlx_read_full_picture(subpage_0, subpage_1, .97, -8, &last_wake_time) != 0)
        {
            ESP_LOGE(TAG, "Failed to read the full picture");
            // Synchronization shifts for one frame
            MLX90640_SynchFrame(MLX90640_SLAVE_ADR);
            continue;
        }
        if (mlx_merge_subpages(subpage_0, subpage_1) != 0)
        {
            ESP_LOGE(TAG, "Failed to merge subpages");
            return;
        }

    
        send_frame_over_uart(subpage_0);

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

    // Print the temperatures
    free(subpage_0);
    free(subpage_1);

    ESP_ERROR_CHECK(i2c_del_master_bus(master_bus_handle));
}
