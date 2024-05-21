#include "mlx90640_i2c_driver.h"
#include "mlx90640_api.h"
#include "freertos/FreeRTOS.h"

paramsMLX90640 mlx90640_type = {
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

int mlx_read_extract_eeprom();

void app_main(void)
{
    // Initialize the I2C bus
    ESP_LOGI(TAG, "Initializing I2C bus");
    MLX90640_I2CInit();

    // Read and extract EEPROM data
    if (mlx_read_extract_eeprom() != 0)
    {
        ESP_LOGE(TAG, "Failed to read and extract EEPROM data");
        return;
    }

    // allocate frame_data buffer
    uint16_t *frame_data = (uint16_t *)malloc(834 * sizeof(uint16_t));
    int frame_number = -1;
    bool both_frames_read = false;

    while (!both_frames_read)
    MLX90640_I2CGeneralReset();

    frame_number = MLX90640_GetFrameData(MLX90640_SLAVE_ADR, frame_data);
    if (frame_number == -1)
    {
        ESP_LOGE(TAG, "Failed to read the frame! Exiting with GetFrameData status -1");
        return;
    }
    else
    {
        ESP_LOGI(TAG, "Frame read successfully! Frame number: %d", frame_number);
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);

    // Calculate ambient temperature (Ta)
    float Ta = MLX90640_GetTa(frame_data, &mlx90640_type);
    ESP_LOGI(TAG, "Ambient temperature: %.2f °C", Ta);

    float emissivity = 0.95;
    float tr = Ta - 8.0;
    float mlx90640To[768] = {0};
    MLX90640_CalculateTo(frame_data, &mlx90640_type, emissivity, tr, mlx90640To);

    // Correct the broken or missing pixel values
    int mode = MLX90640_GetCurMode(MLX90640_SLAVE_ADR);
    MLX90640_BadPixelsCorrection(mlx90640_type.brokenPixels, mlx90640To, mode, &mlx90640_type);
    MLX90640_BadPixelsCorrection(mlx90640_type.outlierPixels, mlx90640To, mode, &mlx90640_type);

    // Print the temperatures
    for (int i = 0; i < 768; i++)
    {
        printf("%.1f;", mlx90640To[i]);
    }

    ESP_ERROR_CHECK(i2c_del_master_bus(master_bus_handle));
}

int mlx_read_extract_eeprom()
{
    // create a temporary eeprom_dump of uint16_t type that will get deleted afterwards
    uint16_t *eeprom_dump = (uint16_t *)malloc(834 * sizeof(uint16_t));
    if (eeprom_dump == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for eeprom_dump");
        return 1;
    }

    // Dump EEPROM data
    if (MLX90640_DumpEE(MLX90640_SLAVE_ADR, eeprom_dump) != 0)
    {
        ESP_LOGE(TAG, "Failed to dump EEPROM data");
        free(eeprom_dump);
        return 2;
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);

    // Extract EEPROM data
    if (MLX90640_ExtractParameters(eeprom_dump, &mlx90640_type) != 0)
    {
        ESP_LOGE(TAG, "Failed to extract EEPROM data");
        free(eeprom_dump);
        return 3;
    }

    return 0;
}

int mlx_read_extract_eeprom()
{
    // create a temporary eeprom_dump of uint16_t type that will get deleted afterwards
    uint16_t *eeprom_dump = (uint16_t *)malloc(834 * sizeof(uint16_t));
    if (eeprom_dump == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for eeprom_dump");
        return 1;
    }

    // Dump EEPROM data
    if (MLX90640_DumpEE(MLX90640_SLAVE_ADR, eeprom_dump) != 0)
    {
        ESP_LOGE(TAG, "Failed to dump EEPROM data");
        free(eeprom_dump);
        return 2;
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);

    // Extract EEPROM data
    if (MLX90640_ExtractParameters(eeprom_dump, &mlx90640_type) != 0)
    {
        ESP_LOGE(TAG, "Failed to extract EEPROM data");
        free(eeprom_dump);
        return 3;
    }

    return 0;
}

int mlx_get_frame_data(uint16_t *frame_data)
{
    int frame_number = -1;
    MLX90640_I2CGeneralReset();
    frame_number = MLX90640_GetFrameData(MLX90640_SLAVE_ADR, frame_data);
    if (frame_number == -1)
    {
        ESP_LOGE(TAG, "Failed to read the frame! Exiting with GetFrameData status -1");
        return;
    }
    else
    {
        ESP_LOGI(TAG, "Frame read successfully! Frame number: %d", frame_number);
    }
    return frame_number;
}

int mlx_transform_frame(uint16_t *frame_data)
{
    vTaskDelay(100 / portTICK_PERIOD_MS);

    // Calculate ambient temperature (Ta)
    float Ta = MLX90640_GetTa(frame_data, &mlx90640_type);
    ESP_LOGI(TAG, "Ambient temperature: %.2f °C", Ta);

    float emissivity = 0.95;
    float tr = Ta - 8.0;
    float mlx90640To[768] = {0};
    MLX90640_CalculateTo(frame_data, &mlx90640_type, emissivity, tr, mlx90640To);

    // Correct the broken or missing pixel values
    int mode = MLX90640_GetCurMode(MLX90640_SLAVE_ADR);
    MLX90640_BadPixelsCorrection(mlx90640_type.brokenPixels, mlx90640To, mode, &mlx90640_type);
    MLX90640_BadPixelsCorrection(mlx90640_type.outlierPixels, mlx90640To, mode, &mlx90640_type);
}