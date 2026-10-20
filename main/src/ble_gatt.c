#include "ble_gatt.h"

static const char *INFO = "GATT";

static uint16_t service_end_handle;
static uint16_t service_start_handle;
static uint16_t cccd_handle;

static int descriptor_discovered(uint16_t conn_handle, const struct ble_gatt_error *error,
      uint16_t chr_def_handle, const struct ble_gatt_dsc *dsc, void *arg);
static int characteristic_discovered(uint16_t conn_handle, const struct ble_gatt_error *error,
      const struct ble_gatt_chr *chr, void *arg);
static int service_discovered(uint16_t conn_handle, const struct ble_gatt_error *error, 
      const struct ble_gatt_svc *service, void *arg);
static int subscribe_complete(uint16_t conn_handle, const struct ble_gatt_error *error, 
      struct ble_gatt_attr*attr, void *arg);


static int descriptor_discovered(uint16_t conn_handle, const struct ble_gatt_error *error,
      uint16_t chr_def_handle, const struct ble_gatt_dsc *dsc, void *arg){
      if (error->status == 0 && dsc != NULL) {
            if (ble_uuid_cmp(&dsc->uuid.u,
                  BLE_UUID16_DECLARE(BLE_GATT_DSC_CLT_CFG_UUID16)) == 0) {

                  uint8_t value[2] = {0x01, 0x00}; // Enable notifications
                  cccd_handle = dsc->handle;
                  ESP_LOGI(INFO, "CCCD found; handle=%d", cccd_handle);

                  int rc = ble_gattc_write_flat(conn_handle, cccd_handle, value,
                        sizeof(value), subscribe_complete, NULL);

                  if (rc != 0) {
                        ESP_LOGE(INFO, "Error: Failed to write CCCD; rc=%d", rc);
                  }
            }
      } else if (error->status == BLE_HS_EDONE) {
            if (cccd_handle == 0) {
                  ESP_LOGE(INFO, "CCCD not found for characteristic");
            }
      } else {
            ESP_LOGE(INFO, "Error discovering descriptor; status=%d att_handle=%d",
                  error->status, error->att_handle);
      }

      return 0;
}

static int characteristic_discovered(uint16_t conn_handle, const struct ble_gatt_error *error,
      const struct ble_gatt_chr *chr, void *arg){
                if (error->status == 0 && chr != NULL) {

        ESP_LOGI(INFO, "Characteristic found; val_handle=%u",
                 chr->val_handle);

        if (!(chr->properties & BLE_GATT_CHR_F_NOTIFY)) {
            ESP_LOGE(INFO, "Characteristic does not support Notify");
            return 0;
        }

        cccd_handle = 0;

        int rc = ble_gattc_disc_all_dscs(
            conn_handle,
            chr->val_handle,
            service_end_handle,
            descriptor_discovered,
            NULL
        );

        if (rc != 0) {
            ESP_LOGE(INFO, "Failed to start descriptor discovery; rc=%d",
                     rc);
        }

    } else if (error->status == BLE_HS_EDONE) {
        ESP_LOGI(INFO, "Characteristic discovery complete");
    } else {
        ESP_LOGE(INFO, "Characteristic discovery failed; status=%d",
                 error->status);
    }

    return 0;
}

static int service_discovered(uint16_t conn_handle, const struct ble_gatt_error *error, 
      const struct ble_gatt_svc *service, void *arg){
            if(error->status == 0 && service != NULL){
                  ESP_LOGI(INFO, "Service found");
                  ESP_LOGI(INFO, "Start handle: %d", service->start_handle);
                  service_start_handle = service->start_handle;
                  ESP_LOGI(INFO, "End handle: %d", service->end_handle);
                  service_end_handle = service->end_handle;
            } else if (error->status == BLE_HS_EDONE){
                  ESP_LOGI(INFO, "Service discovery complete");
                  int rc =ble_gattc_disc_chrs_by_uuid(conn_handle, service_start_handle, 
                  service_end_handle, remote_chr_uuid, characteristic_discovered, NULL);

                  if(rc != 0){
                        ESP_LOGE(INFO, "Error: Failed to discover characteristics; rc=%d", rc);
                  }
                  
            } else {
                  ESP_LOGE(INFO, "Error discovering service; status=%d att_handle=%d", error->status, error->att_handle);
            }

            return 0;
      }

static int subscribe_complete(uint16_t conn_handle, const struct ble_gatt_error *error, 
      struct ble_gatt_attr*attr, void *arg){
            if(error->status == 0){
                  ESP_LOGI(INFO, "Notify Subscripton enabled");
            } else {
                  ESP_LOGE(INFO, "CCCD write failed; status=%d att_handle=%d", error->status, error->att_handle);
            }
            return 0;
      }

int ble_gatt_start_discovery(uint16_t conn_handle){
      return ble_gattc_disc_svc_by_uuid(conn_handle, remote_svc_uuid, service_discovered, NULL);
}