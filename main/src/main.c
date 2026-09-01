#include "common.h"
#include "ble_main.h"
#include "ble_host_task.h"

void app_main(void)
{
      esp_err_t ret = nvs_flash_init();
      if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
      }
       
      ble_init();

      nimble_port_freertos_init(ble_host_task);
}
