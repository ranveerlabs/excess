#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "lwip/sockets.h"
#include "csi_rx.h"
#include "motion.h"


#define SSID  CONFIG_ESP_WIFI_SSID
#define PASS  CONFIG_ESP_WIFI_PASSWORD

static const char *T = "csi";
static csi_frame_t fr;
static uint32_t nframes;
static int print_raw = 0;

static esp_netif_t *nif;

static void on_csi(void *ctx, wifi_csi_info_t *info)
{
    if (!info || !info->buf) return;

    // info->buf is imag,real per subcarrier. imag first, not real first
    const int8_t *b = info->buf;
    int n = info->len / 2;
    if (n > CSI_SUBC) n = CSI_SUBC;

    for (int i = 0; i < n; i++) {
        float im = b[i*2];
        float re = b[i*2 + 1];
        fr.amp[i] = sqrtf(re*re + im*im);
    }
    for (int i = 0; i < CSI_LO; i++) fr.amp[i] = 0;
    for (int i = CSI_HI; i < CSI_SUBC; i++) fr.amp[i] = 0;
    fr.amp[32] = 0;

    fr.rssi = info->rx_ctrl.rssi;
    fr.t_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
    nframes++;

    motion_feed(&fr);



    if (print_raw) {
        printf("R,%lu,%d", (unsigned long)fr.t_ms, fr.rssi);
        for (int i = CSI_LO; i < CSI_HI; i++) printf(",%.1f", fr.amp[i]);
        printf("\n");
    }
}

static void pinger(void *arg)
{
    esp_netif_ip_info_t ip;
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in dst = {0};
    dst.sin_family = AF_INET;
    dst.sin_port = htons(9);

    while (1) {
        if (esp_netif_get_ip_info(nif, &ip) == ESP_OK && ip.gw.addr) {
            dst.sin_addr.s_addr = ip.gw.addr;
            char c = 0;
            sendto(s, &c, 1, 0, (struct sockaddr *)&dst, sizeof(dst));
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void ev(void *a, esp_event_base_t base, int32_t id, void *data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) esp_wifi_connect();
    else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(T, "wifi:drop");
        esp_wifi_connect();
    }
    else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(T, "wifi:ok");
        xTaskCreate(pinger, "ping", 4096, NULL, 4, NULL);
    }
}

void csi_rx_start(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    nif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t ic = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&ic));
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, ev, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, ev, NULL, NULL);

    wifi_config_t wc = {0};
    strncpy((char *)wc.sta.ssid, SSID, sizeof(wc.sta.ssid));
    strncpy((char *)wc.sta.password, PASS, sizeof(wc.sta.password));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wc));

    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    wifi_csi_config_t cc = {
        .lltf_en = true,
        .htltf_en = true,
        .stbc_htltf2_en = false,
        .ltf_merge_en = true,
        .channel_filter_en = true,
        .manu_scale = false,
        .shift = 0,
    };
    ESP_ERROR_CHECK(esp_wifi_set_csi_config(&cc));
    ESP_ERROR_CHECK(esp_wifi_set_csi_rx_cb(on_csi, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_csi(true));

    ESP_ERROR_CHECK(esp_wifi_start());
}
