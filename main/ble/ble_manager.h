#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gatt_common_api.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Public functions that actually exist
esp_err_t ble_manager_init(void);

// Task functions
void ble_manager_handle_device_info_confirmation(void* pvParameters);

// Legacy compatibility
void ble_init(void);

#endif // BLE_MANAGER_H