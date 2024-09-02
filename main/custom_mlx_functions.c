#include "custom_mlx_functions.h"

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
 * @return 0 OK
 * @return -1 Failed to allocate memory for eeprom_dump
 * @return -2 Failed to dump EEPROM data
 * @return -3 Failed to extract EEPROM data from dump
 */
int mlx_read_extract_eeprom()
{
    // create a temporary eeprom_dump of uint16_t type that will get deleted afterwards
    uint16_t *eeprom_dump = (uint16_t *)calloc(832, sizeof(uint16_t));
    if (eeprom_dump == NULL)
    {
        return -1;
    }

    // Dump EEPROM data
    if (MLX90640_DumpEE(MLX90640_SLAVE_ADR, eeprom_dump) != 0)
    {
        free(eeprom_dump);
        return -2;
    }

    // Extract EEPROM data
    if (MLX90640_ExtractParameters(eeprom_dump, &mlx90640_params) != 0)
    {
        free(eeprom_dump);
        return -3;
    }

    free(eeprom_dump); // Free EEPROM dump memory as soon as it is no longer needed
    return 0;
}

/**
 * @brief Read raw frame data and calculate temperatures.
 *
 * Raw subpage sensor data is read into temp array and then the temperatures stored into the subpage_temps array.
 *
 * @param subpage_temps: pointer to the array of temperatures (768 floats)
 * @return frame_number: int 0 or 1
 */
int mlx_get_subpage_temps(float *subpage_temps, float emissivity, int8_t ambient_offset, uint8_t desired_subpage_number, TickType_t *last_wake_time)
{
    const char *TAG = "mlx_get_subpage_temps";
    if (subpage_temps == NULL)
    {
        ESP_LOGE(TAG, "subpage_temps is NULL!");
        return -1;
    }

    uint16_t *subpage_raw_data = (uint16_t *)calloc(834, sizeof(uint16_t));
    if (subpage_raw_data == NULL)
    {
        ESP_LOGE(TAG, "frame_data is NULL!");
        return -2;
    }

    // MLX90640_SynchFrame(MLX90640_SLAVE_ADR);
    int subpage_number = MLX90640_GetFrameData(MLX90640_SLAVE_ADR, subpage_raw_data, last_wake_time);
    if (subpage_number != desired_subpage_number)
    {
        ESP_LOGE(TAG, "Wrong subpage: (wanted: %d, read: %d)", desired_subpage_number, subpage_number);
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
    return subpage_number;
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
    const char *TAG = "mlx_read_full_picture";

    int page_number = -404;

    // Read subpage 0
    page_number = mlx_get_subpage_temps(subpage_temps_0, emissivity, ambient_offset, 0, last_wake_time);
    if (page_number != 0)
    {
        ESP_LOGE(TAG, "Failed to read subpage 0. Error: %d", page_number);
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
    const char *TAG = "mlx_merge_subpages";

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