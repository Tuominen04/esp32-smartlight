#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_ota_ops.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"

#define DEVICE_NUMBER 1

// Function declarations
bool load_device_info(char* out_device_name, size_t name_buf_size, char* out_device_id, size_t id_buf_size);
void device_manager_save_device_info(const char* ssid, const char* password);
void send_device_info_via_ble(void);
esp_app_desc_t* get_firmware_info(void);

// Callback function type for setting BLE handles
typedef void (*set_device_info_handle_cb_t)(uint16_t handle);
void device_info_set_ble_info(esp_gatt_if_t gatts_if, uint16_t conn_id);
void device_info_set_ble_handle(uint16_t handle);

#endif // DEVICE_INFO_H