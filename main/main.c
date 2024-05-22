#include "mlx90640_i2c_driver.h"
#include "mlx90640_api.h"
#include "freertos/FreeRTOS.h"

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

// int mlx_read_extract_eeprom(paramsMLX90640 *);
// int mlx_read_frames(uint16_t *);
// int mlx_calculate_temperatures(uint16_t *, float *, float, float);

void app_main(void)
{
    // Initialize the I2C bus
    ESP_LOGI(TAG, "Initializing I2C bus");
    MLX90640_I2CInit();


    MLX90640_SetRefreshRate(MLX90640_SLAVE_ADR, 0x02);

    // Allocate memory for EEPROM dump
    uint16_t *eeprom_dump = (uint16_t *)malloc(834 * sizeof(uint16_t));
    if (eeprom_dump == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for eeprom_dump");
        return;
    }

    // Dump EEPROM data
    if (MLX90640_DumpEE(MLX90640_SLAVE_ADR, eeprom_dump) != 0)
    {
        ESP_LOGE(TAG, "Failed to dump EEPROM data");
        free(eeprom_dump);
        return;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);

    // Extract EEPROM data
    paramsMLX90640 mlx90640_params;
    if (MLX90640_ExtractParameters(eeprom_dump, &mlx90640_params) != 0)
    {
        ESP_LOGE(TAG, "Failed to extract EEPROM data");
        free(eeprom_dump);
        return;
    }

    free(eeprom_dump); // Free EEPROM dump memory as soon as it is no longer needed

    // Allocate memory for frame data
    uint16_t *frame_0_data = (uint16_t *)malloc(834 * sizeof(uint16_t));
    uint16_t *frame_1_data = (uint16_t *)malloc(834 * sizeof(uint16_t));
    if (frame_0_data == NULL || frame_1_data == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for frame data");
        free(frame_0_data);
        free(frame_1_data);
        return;
    }

    uint16_t *frame_pointer = frame_0_data;
    int frame_number = -1;
    bool frame_0_read = false;
    bool frame_1_read = false;
    // MLX90640_I2CGeneralReset();
    MLX90640_SynchFrame(MLX90640_SLAVE_ADR);
    while (!frame_0_read || !frame_1_read)
    {
        frame_number = MLX90640_GetFrameData(MLX90640_SLAVE_ADR, frame_pointer);
        if (frame_number == -1)
        {
            ESP_LOGE(TAG, "Failed to read the frame! Exiting with GetFrameData status -1");
            free(frame_0_data);
            free(frame_1_data);
            return;
        }
        if (frame_number == 0)
        {
            ESP_LOGI(TAG, "Frame 0 ready");
            frame_0_read = true;
            frame_pointer = frame_1_data;
        }
        else if (frame_number == 1)
        {
            ESP_LOGI(TAG, "Frame 1 ready");
            if (!frame_0_read)
            {
                ESP_LOGI(TAG, "Skipping frame 1");
                continue;
            }
            frame_1_read = true;
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }

    float emissivity = 0.99;
    int8_t tr_offset = 6;

    // Process frame 0
    float Ta_0 = MLX90640_GetTa(frame_0_data, &mlx90640_params);
    float tr = Ta_0 - tr_offset; // Ambient temperature offset
    ESP_LOGI(TAG, "Frame 0 ambient temp: %.2f °C (%.1f)", Ta_0, tr);
    float mlx90640To_0[768] = {0};
    MLX90640_CalculateTo(frame_0_data, &mlx90640_params, emissivity, tr, mlx90640To_0);
    free(frame_0_data); // Free frame 0 data memory as soon as it is no longer needed

    // Correct the broken or missing pixel values for frame 0
    int mode = MLX90640_GetCurMode(MLX90640_SLAVE_ADR);
    MLX90640_BadPixelsCorrection(mlx90640_params.brokenPixels, mlx90640To_0, mode, &mlx90640_params);
    MLX90640_BadPixelsCorrection(mlx90640_params.outlierPixels, mlx90640To_0, mode, &mlx90640_params);

    // Process frame 1
    float mlx90640To_1[768] = {0};
    float Ta_1 = MLX90640_GetTa(frame_1_data, &mlx90640_params);
    tr = Ta_1 - tr_offset; // Ambient temperature offset
    ESP_LOGI(TAG, "Frame 1 ambient temp: %.2f °C (%.1f)", Ta_0, tr);

    MLX90640_CalculateTo(frame_1_data, &mlx90640_params, emissivity, tr, mlx90640To_1);
    free(frame_1_data); // Free frame 1 data memory as soon as it is no longer needed

    // Correct the broken or missing pixel values for frame 1
    MLX90640_BadPixelsCorrection(mlx90640_params.brokenPixels, mlx90640To_1, mode, &mlx90640_params);
    MLX90640_BadPixelsCorrection(mlx90640_params.outlierPixels, mlx90640To_1, mode, &mlx90640_params);

    // Combine the two frames (for averaging)
    for (int i = 0; i < 768; i++)
    {
        mlx90640To_0[i] = mlx90640To_0[i] + mlx90640To_1[i];
    }

    // Print the temperatures
    for (int i = 0; i < 768; i++)
    {
        printf("%.1f;", mlx90640To_0[i]);
    }

    ESP_ERROR_CHECK(i2c_del_master_bus(master_bus_handle));
}

// /**
//  * @brief Read and extract EEPROM data.
//  *
//  * Read the EEPROM data and extract the parameters.
//  * The extracted parameters are stored in paramsMLX90640 struct.
//  *
//  * @return int
//  */
// int mlx_read_extract_eeprom(paramsMLX90640 *mlx90640_type)
// {
//     // create a temporary eeprom_dump of uint16_t type that will get deleted afterwards
//     uint16_t *eeprom_dump = (uint16_t *)malloc(834 * sizeof(uint16_t));
//     if (eeprom_dump == NULL)
//     {
//         ESP_LOGE(TAG, "Failed to allocate memory for eeprom_dump");
//         return 1;
//     }

//     // Dump EEPROM data
//     if (MLX90640_DumpEE(MLX90640_SLAVE_ADR, eeprom_dump) != 0)
//     {
//         ESP_LOGE(TAG, "Failed to dump EEPROM data");
//         free(eeprom_dump);
//         return 2;
//     }
//     vTaskDelay(50 / portTICK_PERIOD_MS);

//     // Extract EEPROM data
//     if (MLX90640_ExtractParameters(eeprom_dump, mlx90640_type) != 0)
//     {
//         ESP_LOGE(TAG, "Failed to extract EEPROM data");
//         free(eeprom_dump);
//         return 3;
//     }

//     free(eeprom_dump); // Free allocated memory after use
//     return 0;
// }

// /**
//  * @brief Read frames until both frames are read successfully.
//  *
//  * Loop keeps reading until frame 0 and then frame 1 are read successfully.
//  * Loop demands frame 0 first and frame 1 second.
//  * If 10 subsequent failed attempts fail, the function returns -1.
//  *
//  * @param frame_data: pointer to the frame_data[834]. Must at least 834!
//  * @return int
//  */
// int mlx_read_frames(uint16_t *frame_data)
// {
//     int frame_number = -1;
//     bool frame_0_read = false;
//     uint8_t failed_attempts = 0;

//     MLX90640_I2CGeneralReset();
//     frame_number = MLX90640_GetFrameData(MLX90640_SLAVE_ADR, frame_data);

//     return 0;
//     // while (1)
//     // {
//     //     frame_number = -1;
//     //     MLX90640_I2CGeneralReset();
//     //     frame_number = MLX90640_GetFrameData(MLX90640_SLAVE_ADR, frame_data);

//     //     return 0;

//     //     // if (frame_number == -1)
//     //     // {
//     //     //     failed_attempts++;
//     //     //     if (failed_attempts >= 10)
//     //     //     {
//     //     //         ESP_LOGE(TAG, "Failed to read the frame %d subsequent times!", failed_attempts);
//     //     //         return -1;
//     //     //     }
//     //     // }
//     //     // else
//     //     //     return 0;
//     //     // {
//     //     //     failed_attempts = 0;

//     //     //     if (frame_number == 0)
//     //     //     {
//     //     //         frame_0_read = true;
//     //     //     }
//     //     //     else if (frame_number == 1)
//     //     //     {
//     //     //         if (!frame_0_read)
//     //     //             continue;
//     //     //         return 0;
//     //     //     }
//     //     // }
//     // }
// }

// /**
//  * @brief Transform raw frame data to temperatures. You must first read eeprom and read frames.
//  *
//  * Calculate the ambient temperature (Ta) and then calculate the temperatures of all pixels.
//  * Correct the broken or missing pixel values (boken and outlier pixels are defined in mlx90640_type)
//  *
//  * @param raw_data_array: pointer to the array of raw pixels data (at least 834 long)
//  * @param temps_data_array: pointer to the array with temperatures (at least 768 long)
//  * @param emissivity: emissivity of the object (0.0 - 1.0, usually 0.95)
//  * @param ambient_temp_offset: ambient temperature offset (adjust based on the sensor's environment)
//  * @return int
//  */
// int mlx_calculate_temperatures(uint16_t *raw_data_array, float *temps_data_array, float emissivity, float ambient_temp_offset)
// {
//     if (raw_data_array == NULL)
//     {
//         ESP_LOGE(TAG, "raw_frame_data is NULL!");
//         return 1;
//     }
//     if (temps_data_array == NULL)
//     {
//         ESP_LOGE(TAG, "frame_data_temperatures is NULL!");
//         return 2;
//     }
//     float Ta = MLX90640_GetTa(raw_data_array, &mlx90640_type) - ambient_temp_offset;
//     MLX90640_CalculateTo(raw_data_array, &mlx90640_type, emissivity, Ta, temps_data_array);

//     // Correct the broken or missing pixel values
//     int mode = MLX90640_GetCurMode(MLX90640_SLAVE_ADR);
//     MLX90640_BadPixelsCorrection(mlx90640_type.brokenPixels, temps_data_array, mode, &mlx90640_type);
//     MLX90640_BadPixelsCorrection(mlx90640_type.outlierPixels, temps_data_array, mode, &mlx90640_type);

//     return 0;
// }