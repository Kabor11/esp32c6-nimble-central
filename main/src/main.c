#include "common.h"
#include "ble_main.h"
#include "ble_host_task.h"
#include "wifi_main.h"
#include "mqtt.h"

static const char* TAG = "main";

void app_main(void)
{
      esp_err_t ret = nvs_flash_init();
      if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
      }
      
      ESP_LOGI(TAG, "Starting BLE initialization");
      ble_init();
      nimble_port_freertos_init(ble_host_task);
      ESP_LOGI(TAG, "BLE initialization completed");

      ESP_LOGI(TAG, "Starting WiFi initialization");
      wifi_init();
      ESP_LOGI(TAG, "WiFi initialization completed");

      ESP_LOGI(TAG, "Starting MQTT initialization");
      mqtt_app_start();
      ESP_LOGI(TAG, "MQTT initialization completed");
}
