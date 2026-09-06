#include "ble_main.h"
#include "ble_gap.h"

static const char* TAG = "ble_main";

void ble_init(void){
      esp_err_t ret = nimble_port_init();
      if(ret != ESP_OK){
            ESP_LOGE(TAG, "Error: Failed to initialize NimBLE; ret=%d", ret);
            return;
      }

      ble_hs_cfg.reset_cb = ble_on_reset;
      ble_hs_cfg.sync_cb = ble_on_sync;
      ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

      int rc = ble_svc_gap_device_name_set("Robak NimBLE Central");
      if(rc != 0){
            ESP_LOGE(TAG, "Error: Failed to set device name; rc=%d", rc);
            return;
      }
}