#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "csi_rx.h"
#include "motion.h"

#define LED 48      // s3 devkit rgb, im only using it as a plain gpio

void app_main(void)
{
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    csi_rx_start();

    int last = -1;
    while (1) {
        int p = motion_state();
        gpio_set_level(LED, p);
        if (p != last) {
            printf("S,%d,%.2f\n", p, motion_score());
            last = p;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
