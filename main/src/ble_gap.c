#include "ble_gap.h"
//trzeba będzie dodać uuid urządzenia (svc i charakterystyki) z którego będzie odbierany notify

// BLE_GAP_EVENT_DISC
// BLE_GAP_EVENT_CONNECT
// BLE_GAP_EVENT_DISCONNECT
// BLE_GAP_EVENT_NOTIFY_RX

static const ble_uuid_t *remote_svc_uuid =
    BLE_UUID128_DECLARE(
        0x7b, 0x4e, 0x2c, 0x91,
        0x6a, 0x53, 0x4f, 0x18,
        0x9d, 0x72, 0x31, 0xc8,
        0xa6, 0xe5, 0x04, 0xbf
    );

int ble_gap_event(struct ble_gap_event *event, void *arg){
      struct ble_gap_conn_desc desc;
      struct ble_hs_adv_fields fields;

      int rc;

      switch (event->type)
      {
      case BLE_GAP_EVENT_DISC:
            rc = ble_hs_adv_parse_fields(&fields, event->disc.data, event->disc.length_data);
            if(rc != 0){
                  return 0;
            }

            
            break;
      
      default:
            break;
      }
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

      ble_gap_disc_cancel(); //dopisać

      ble_hs_id_infer_auto(0, &addr_type);

      ble_gap_connect(addr_type, &disc->addr, 30000, NULL, ble_gap_event, NULL);
}