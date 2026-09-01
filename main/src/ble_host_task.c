#include "ble_host_task.h"

static const char* TAG = "ble_host_task";

void ble_host_task(void *param){
      ESP_LOGI(TAG, "BLE Host Task started");

      nimble_port_run(); // This function will block and run the BLE host task
      nimble_port_freertos_deinit();
}