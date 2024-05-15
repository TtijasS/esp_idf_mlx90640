#include "constants.h"
#include "mlx90640.h"
#include "my_i2c_config.h"
#include "data_structs.h"
#include "custom_functions.h"

void app_main(void)
{
    ESP_LOGI(TAG, "Example started");
    // Setup master handles and initialize I2C
    init_i2c();

    // Setup the MPU6050 registers
    // mlx_initial_setup(&i2c_buffer);

    if (mlx_read_ram_data(&i2c_buffer))
    {
        ESP_LOGI(TAG, "Data read from the MLX90640 RAM");
    }

    printf("Data read from the MLX90640 RAM:\n");
    uint16_t pixel_val = 0;
    for (int i = 0; i < I2C_READ_BUFF_SIZE; i += 2)
    {
        pixel_val = (i2c_buffer.read_buffer[i] << 8) | i2c_buffer.read_buffer[i + 1];
        printf("%u | ", pixel_val);
        if ((i / 2 + 1) % 16 == 0)
        { // Print newline after every 16 pixels
            printf("\n");
        }
    }
    ESP_ERROR_CHECK(i2c_del_master_bus(master_bus_handle));
}