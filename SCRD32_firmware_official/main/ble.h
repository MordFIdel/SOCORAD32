#ifndef BLE_H
#define BLE_H

enum
{
    IDX_SVC,
    IDX_CHAR_A,
    IDX_CHAR_VAL_A,
    IDX_CHAR_CFG_A,

    IDX_CHAR_B,
    IDX_CHAR_VAL_B,
    IDX_CHAR_CFG_B,

    IDX_CHAR_C,
    IDX_CHAR_VAL_C,
    IDX_CHAR_CFG_C,
    IDX_CHAR_CFG_C_2,

    HRS_IDX_NB,
};

void ble_init(void);
void sendResponse(esp_gatt_if_t gatts_if, uint16_t conn_id, uint8_t *data, uint16_t len);

#endif