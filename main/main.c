#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "constants.h"
#include "app_tasks.h"

const char *TAG = "app_main";

// ----------------- Main -----------------
void app_main(void)
{
    ESP_LOGE(TAG, "LOG LVL ERROR");
    ESP_LOGW(TAG, "LOG LVL WARNING");
    ESP_LOGI(TAG, "LOG LVL INFO");
    ESP_LOGD(TAG, "LOG LVL DEBUG");
    if (xTaskCreatePinnedToCore(task_initialization, "Init task", TASK_INIT_STACK_SIZE, NULL, 10, NULL, tskNO_AFFINITY) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create init task");
    }
}
