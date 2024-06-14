#include "mlx90640_i2c_driver.h"
#include "mlx90640_api.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include <stdio.h>

paramsMLX90640 mlx90640_params = {
    .kVdd = 0,
    .vdd25 = 0,
    .KvPTAT = 0,
    .KtPTAT = 0,
    .vPTAT25 = 0,
    .alphaPTAT = 0,
    .gainEE = 0,
    .tgc = 0,
    .cpKv = 0,
    .cpKta = 0,
    .resolutionEE = 0,
    .calibrationModeEE = 0,
    .KsTa = 0,
    .ksTo = {0},
    .ct = {0},
    .alpha = {0},
    .alphaScale = 0,
    .offset = {0},
    .kta = {0},
    .ktaScale = 0,
    .kv = {0},
    .kvScale = 0,
    .cpAlpha = {0},
    .cpOffset = {0},
    .ilChessC = {0},
    .brokenPixels = {0},
    .outlierPixels = {0},
};

// ----------------- Prototypes -----------------
void mlx_delay_after_por();
void mlx_delay_before_subpage();
int mlx_read_extract_eeprom();
int mlx_get_subpage_temps(float *, float, int8_t, uint8_t, TickType_t *);
int mlx_read_full_picture(float *, float *, float , int8_t, TickType_t *);
int mlx_merge_subpages(float *, float *);

// ----------------- Constants -----------------
const int DELAY_BETWEEN_SUBPAGES = MLX_REFRESH_MILLIS * 0.9;


// ----------------- Main -----------------
void app_main(void)
{
    // Example of counting time with vTaskDelay and xTaskDelayUntil
    // printf("port tick period ms: %ld\n", pdTICKS_TO_MS(1));
    // TickType_t last_wake_time = xTaskGetTickCount();
    // TickType_t wake_time = xTaskGetTickCount();
    // TickType_t deltatime_ = 0;
    // for (int i = 0; i < 10; i++)
    // {
    //     vTaskDelay(pdMS_TO_TICKS(500));
    //     deltatime_ = pdTICKS_TO_MS(xTaskGetTickCount() - wake_time);
    //     printf("Deltatime: %ld\n", deltatime_);
    //     xTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(1000));
    //     deltatime_ = pdTICKS_TO_MS(xTaskGetTickCount() - wake_time);
    //     // last_wake_time = xTaskGetTickCount();
    //     printf("Deltatime: %ld\n", deltatime_);
    //     last_wake_time = xTaskGetTickCount();
    //     wake_time = xTaskGetTickCount();
    // }
    // return;

    // Example of counting time with esp_timer (1 microsecond precision)
    // uint64_t start_time = esp_timer_get_time();
    // for (int i = 0; i < 10; i++)
    // {
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
    // uint64_t end_time = esp_timer_get_time();
    // printf("Deltatime: %lld\n", (end_time - start_time) / 1000);
    // return;
    // for(int i = 0; i < 20; i++)
    // {
    //     // take current time and pring out deltatime at the end
    //     uint64_t start_time = esp_timer_get_time();
    //     // mlx_delay_before_subpage();
    //     vTaskDelay(800 / portTICK_PERIOD_MS);
    //     uint64_t end_time = esp_timer_get_time();
    //     printf("%lld\n", (end_time - start_time) / 1000);
    // }
    // return;
    // Initialize the I2C bus
    ESP_LOGI(TAG, "Initializing I2C bus");
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
    float *subpage_0 = (float *)malloc(768 * sizeof(float));
    float *subpage_1 = (float *)malloc(768 * sizeof(float));
    if ((subpage_0 == NULL) | (subpage_1 == NULL))
    {
        ESP_LOGE(TAG, "Failed to allocate memory for frame data");
        return;
    }
    // ##################################################### TESTING FRAME SYNCHRONIZATION
    // uint16_t *subpage_raw_data = (uint16_t *)malloc(834 * sizeof(uint16_t));
    // if (subpage_raw_data == NULL)
    // {
    //     ESP_LOGE(TAG, "frame_data is NULL!");
    // }
    // // Set all values to 0
    // for (int i = 0; i < 834; i++)
    // {
    //     subpage_raw_data[i] = 0;
    // }

    // int frame_number = -1;

    // // Set all to 0
    // MLX90640_SynchFrame(MLX90640_SLAVE_ADR);
    // uint64_t start_time = esp_timer_get_time();
    // uint64_t deltatime = 0;

    // while(1)
    // {
    //     // MLX90640_SynchFrame(MLX90640_SLAVE_ADR);
    //     frame_number = MLX90640_GetFrameData(MLX90640_SLAVE_ADR, subpage_raw_data);
    //     ESP_LOGI(TAG, "Frame number: %d", frame_number);        
    //     deltatime = (esp_timer_get_time() - start_time) / 1000;
    //     ESP_LOGI(TAG, "deltatime: %lld", deltatime);
    //     start_time = esp_timer_get_time();
    // }
    // #####################################################
    MLX90640_SynchFrame(MLX90640_SLAVE_ADR);
    vTaskDelay(pdMS_TO_TICKS(MLX_REFRESH_MILLIS));
    TickType_t last_wake_time = 0;
    TickType_t deltatime = 0;
    int deltatime_diff = 0;
    while (1)
    {
        // Take current tick count
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
            // mlx_delay_before_subpage(); // delay to prevent the missalignment of the subpages
            continue;
        }
        if (mlx_merge_subpages(subpage_0, subpage_1) != 0)
        {
            ESP_LOGE(TAG, "Failed to merge subpages");
            return;
        }

        printf("%02x\n", 0x1F);
        for (int i = 0; i < 768; i++)
        {
            if (i == 767)
            {
                printf("%.2f\n", subpage_0[i]);
                break;
            }
            else
            {
                printf("%.2f;", subpage_0[i]);
            }
        }
        printf("%02x\n", 0xF1);

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

// ----------------- Functions -----------------

/**
 * @brief Delay the correct ammount of time after power on reset.
 *
 */
void mlx_delay_after_por()
{
    vTaskDelay(pdMS_TO_TICKS(MLX_REFRESH_MILLIS * 2 + 80));
}

/**
 * @brief Read and extract EEPROM data.
 *
 * Read the EEPROM data and extract the parameters.
 * The extracted parameters are stored in paramsMLX90640 struct.
 *
 * @return int
 */
int mlx_read_extract_eeprom()
{
    // create a temporary eeprom_dump of uint16_t type that will get deleted afterwards
    uint16_t *eeprom_dump = (uint16_t *)malloc(832 * sizeof(uint16_t));
    if (eeprom_dump == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for eeprom_dump");
        return 1;
    }
    // set all values to 0
    for (int i = 0; i < 832; i++)
    {
        eeprom_dump[i] = 0;
    }

    // Dump EEPROM data
    if (MLX90640_DumpEE(MLX90640_SLAVE_ADR, eeprom_dump) != 0)
    {
        ESP_LOGE(TAG, "Failed to dump EEPROM data");
        free(eeprom_dump);
        return 2;
    }

    // Extract EEPROM data
    if (MLX90640_ExtractParameters(eeprom_dump, &mlx90640_params) != 0)
    {
        ESP_LOGE(TAG, "Failed to extract EEPROM data");
        free(eeprom_dump);
        return 3;
    }

    free(eeprom_dump); // Free EEPROM dump memory as soon as it is no longer needed
    return 0;
}

/**
 * @brief Read raw frame data and calculate temperatures.
 *
 * Raw subpage sensor data is read into temp array and then the temperatures stored into the subpage_temps array.
 *
 * @param subpage_temps: pointer to the array of temperatures (at least 768 long)
 * @return frame_number: int 0 or 1
 */
int mlx_get_subpage_temps(float *subpage_temps, float emissivity, int8_t ambient_offset, uint8_t subpage_number, TickType_t *last_wake_time)
{
    if (subpage_temps == NULL)
    {
        ESP_LOGE(TAG, "subpage_temps is NULL!");
        return -1;
    }

    uint16_t *subpage_raw_data = (uint16_t *)malloc(834 * sizeof(uint16_t));
    if (subpage_raw_data == NULL)
    {
        ESP_LOGE(TAG, "frame_data is NULL!");
        return -2;
    }
    // Set all values to 0
    for (int i = 0; i < 834; i++)
    {
        subpage_raw_data[i] = 0;
    }

    int frame_number = 404;

    // MLX90640_SynchFrame(MLX90640_SLAVE_ADR);
    frame_number = MLX90640_GetFrameData(MLX90640_SLAVE_ADR, subpage_raw_data, last_wake_time);
    if (frame_number != subpage_number)
    {
        ESP_LOGE(TAG, "Wrong subpage: (wanted: %d, read: %d)", subpage_number, frame_number);
        free(subpage_raw_data);
        return -3;
    }

    // Get the ambient temperature
    float ambient_temperature = MLX90640_GetTa(subpage_raw_data, &mlx90640_params);
    ambient_temperature += ambient_offset; // offset the ambient temperature
    // ESP_LOGI(TAG, "Ambient temp: %.2f °C", ambient_temperature);

    // Calculate subpage temperatures
    MLX90640_CalculateTo(subpage_raw_data, &mlx90640_params, emissivity, ambient_temperature, subpage_temps);
    // Free the raw data memory as soon as it is no longer needed
    free(subpage_raw_data);

    // Get the current mode of the sensor
    int mode = MLX90640_GetCurMode(MLX90640_SLAVE_ADR);
    // Correct the broken or missing pixel values
    MLX90640_BadPixelsCorrection(mlx90640_params.brokenPixels, subpage_temps, mode, &mlx90640_params);
    MLX90640_BadPixelsCorrection(mlx90640_params.outlierPixels, subpage_temps, mode, &mlx90640_params);
    return frame_number;
}

/**
 * @brief Read both subpages and calculate the temperatures.
 *
 * @param subpage_temps_0: pointer to the array of subpage temperatures (at least 768 long)
 * @param subpage_temps_1: pointer to the array of subpage temperatures (at least 768 long)
 * @param emissivity: emissivity of the object
 * @param ambient_offset: offset to the ambient temperature
 * @return int
 */
int mlx_read_full_picture(float *subpage_temps_0, float *subpage_temps_1, float emissivity, int8_t ambient_offset, TickType_t *last_wake_time)
{
    // Read the first subpage
    // ESP_LOGI(TAG, "Reading the first subpage");
    int page_number = -404;

    // Read subpage 0
    page_number = mlx_get_subpage_temps(subpage_temps_0, emissivity, ambient_offset, 0, last_wake_time);
    if (page_number != 0 ){
        ESP_LOGE(TAG, "mlx_read_full_picture: Failed to read subpage 0. Error: %d", page_number);
        return -1;
    }

    xTaskDelayUntil(last_wake_time, pdMS_TO_TICKS(DELAY_BETWEEN_SUBPAGES));
    // Read subpage 1
    page_number = mlx_get_subpage_temps(subpage_temps_1, emissivity, ambient_offset, 1, last_wake_time);
    if (page_number != 1)
    {
        ESP_LOGE(TAG, "mlx_read_full_picture: Failed to read subpage 1. Error: %d", page_number);
        return -2;
    }
    return 0;
}

/**
 * @brief Merge both subpages into subpage_temps_0.
 *
 * Temperatures from subpage 0 and 1 are add together and stored in subpage_temps_0.
 *
 * @param subpage_temps_0
 * @param subpage_temps_1
 * @return int
 */
int mlx_merge_subpages(float *subpage_temps_0, float *subpage_temps_1)
{
    if (subpage_temps_0 == NULL || subpage_temps_1 == NULL)
    {
        ESP_LOGE(TAG, "subpage_temps_0 or subpage_temps_1 is NULL!");
        return 1;
    }

    for (int i = 0; i < 768; i++)
    {
        subpage_temps_0[i] += subpage_temps_1[i];
    }

    return 0;
}