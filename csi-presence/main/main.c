#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "csi_rx.h"

void app_main(void)
{
    csi_rx_start();
    while (1) vTaskDelay(pdMS_TO_TICKS(1000));
}
