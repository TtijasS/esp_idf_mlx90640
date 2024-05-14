#include "constants.h"
#include "mlx90640.h"
#include "my_i2c_config.h"
#include "data_structs.h"
#include "custom_functions.h"


void app_main(void)
{
    // Setup master handles and initialize I2C
    init_i2c();

    // Setup the MPU6050 registers
    mlx_initial_setup(&i2c_buffer);

    // mpu_fifo_reset(&i2c_buffer);
    // while (1)
    // {
    //     // Read the data from the MPU6050
        // vTaskDelay(10 / portTICK_PERIOD_MS);
    // }
    // ESP_ERROR_CHECK(i2c_del_master_bus(master_bus_handle));
}