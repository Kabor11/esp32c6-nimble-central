#include "ble_gap.h"
#include "ble_gatt.h"
//trzeba będzie dodać uuid urządzenia (svc i charakterystyki) z którego będzie odbierany notify


//jest napisane wszystko oprócz oprócz połączenia gap i subskrypcji

static const char* TAG = "ble_gap";


int ble_gap_event(struct ble_gap_event *event, void *arg){
      struct ble_gap_conn_desc desc;
      struct ble_hs_adv_fields fields;

      int rc;

      switch (event->type){
      case BLE_GAP_EVENT_DISC:
            rc = ble_hs_adv_parse_fields(&fields,
                                          event->disc.data,
                                          event->disc.length_data);

            if (rc != 0) {
                  ESP_LOGE(TAG, "Failed to parse advertisement; rc=%d", rc);
                  return 0;
            }

            ESP_LOGI(TAG, "--------------------------------");
            ESP_LOGI(TAG, "Device found");
            ESP_LOGI(TAG, "RSSI: %d dBm", event->disc.rssi);

            // tutaj wypisujesz adres BLE
            ESP_LOGI(TAG, "Address: %02X:%02X:%02X:%02X:%02X:%02X",
                  event->disc.addr.val[5],
                  event->disc.addr.val[4],
                  event->disc.addr.val[3],
                  event->disc.addr.val[2],
                  event->disc.addr.val[1],
                  event->disc.addr.val[0]);
            // tutaj wypisujesz nazwę, jeżeli advertising ją zawiera
            ESP_LOGI(TAG, "name_len = %d", fields.name_len);
            ESP_LOGI(TAG, "Device name: %.*s", fields.name_len, fields.name);
            // tutaj możesz wypisać UUID-y
            ESP_LOGI(TAG, "Number of 128-bit UUIDs: %d", fields.num_uuids128);
            ESP_LOGI(TAG, "--------------------------------");

            connect_if_interesting(&event->disc);
            
            return 0;
      case BLE_GAP_EVENT_CONNECT:
            if(event->connect.status == 0){
                  ESP_LOGI(TAG, "Connected conn_handle=%d", event->connect.conn_handle);

                  rc = ble_gatt_start_discovery(event->connect.conn_handle);

                  if(rc != 0){
                        ESP_LOGE(TAG, "Error: Failed to start discovery; rc=%d", rc);
                        
                        ble_gap_terminate(event->connect.conn_handle, BLE_ERR_REM_USER_CONN_TERM);
                        return 0;
                  }
            } else {
                  ESP_LOGE(TAG, "Connection failed; status=%d", event->connect.status);

                  central_scan();
            }
            return 0;

      case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(TAG, "Disconnected; reason=%d", event->disconnect.reason);

            central_scan();
            return 0;

      case BLE_GAP_EVENT_NOTIFY_RX:
            uint8_t buf[32]; //na potrzeby esp_logi
            uint16_t len; //na potrzeby esp_logi
            ESP_LOGI(TAG, "Received notification; conn_handle=%d attr_handle=%d len=%d",
                  event->notify_rx.conn_handle,
                  event->notify_rx.attr_handle,
                  OS_MBUF_PKTLEN(event->notify_rx.om));

            rc = ble_hs_mbuf_to_flat(event->notify_rx.om, buf, sizeof(buf), &len);
            if(rc != 0){
                  ESP_LOGE(TAG, "Error: Failed to copy notification data; rc=%d", rc);
                  return 0;
            }
            ESP_LOGI(TAG, "Received data: ");

            for (int i = 0; i < len; i++) {
                  ESP_LOGI(TAG, "%02X ", buf[i]);
            }

            return 0;
      
      default:
            break;
      }
      return 0;
}


// ============================================================================
//
// Kod pomocniczy
//
// ============================================================================

int ble_should_connect(const struct ble_gap_disc_desc *disc){
      struct ble_hs_adv_fields fields;
      int rc;

      if(disc->event_type != BLE_HCI_ADV_RPT_EVTYPE_ADV_IND &&
         disc->event_type != BLE_HCI_ADV_RPT_EVTYPE_DIR_IND){
            return 0;
      }
      
      rc = ble_hs_adv_parse_fields(&fields, disc->data, disc->length_data);
      if(rc != 0){
            return 0;
      }

      for(int i = 0; i < fields.num_uuids128; i++){
            if(ble_uuid_cmp(&fields.uuids128[i].u, remote_svc_uuid) == 0){
                  return 1;
            }
      }
      return 0;
}

void connect_if_interesting(const struct ble_gap_disc_desc *disc){
      uint8_t addr_type;
      if(!ble_should_connect(disc)){
            return;
      } 

      ble_gap_disc_cancel();

      ble_hs_id_infer_auto(0, &addr_type);

      ble_gap_connect(addr_type, &disc->addr, 30000, NULL, ble_gap_event, NULL);
}

void central_scan(void){
      uint8_t own_addr_type;
      struct ble_gap_disc_params disc_params = {0};
      int rc;

      rc = ble_hs_id_infer_auto(0, &own_addr_type);
      if(rc != 0){
            ESP_LOGE(TAG, "Error: Failed to determine address type; rc=%d", rc);
            return;
      }

      disc_params.passive = 1;
      disc_params.filter_duplicates = 1;
      disc_params.itvl = 0;
      disc_params.window = 0;
      disc_params.filter_policy = 0;
      disc_params.limited = 0;

      rc = ble_gap_disc(own_addr_type, BLE_HS_FOREVER, &disc_params, ble_gap_event, NULL);
      if(rc != 0){
            ESP_LOGE(TAG, "Error: Failed to start scanning; rc=%d", rc);
            return;
      }

}

void ble_on_reset(int reason){
      ESP_LOGE(TAG, "Resetting state; reason=%d", reason);

}
void ble_on_sync(void){
      int rc;
      rc = ble_hs_util_ensure_addr(0);
      assert(rc == 0);

      central_scan();
}