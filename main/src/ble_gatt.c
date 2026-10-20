#include "ble_gatt.h"

static const char *INFO = "GATT";

static int service_discovered(uint16_t conn_handle, const struct ble_gatt_error *error, 
      const struct ble_gatt_svc *service, void *arg){
            if(error->status == 0 && service != NULL){
                  ESP_LOGI(INFO, "Service found");
                  ESP_LOGI(INFO, "Start handle: %d", service->start_handle);
                  ESP_LOGI(INFO, "End handle: %d", service->end_handle);
            } else if (error->status == BLE_HS_EDONE){
                  ESP_LOGI(INFO, "Service discovery complete");
            } else {
                  ESP_LOGE(INFO, "Error discovering service; status=%d att_handle=%d", error->status, error->att_handle);
            }

            return 0;
      }

int ble_gatt_start_discovery(uint16_t conn_handle){
      return ble_gattc_disc_svc_by_uuid(conn_handle, remote_svc_uuid, service_discovered, NULL);
}