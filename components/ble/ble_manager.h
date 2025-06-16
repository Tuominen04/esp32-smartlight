/**
 * @file ble_manager.h
 * @brief BLE manager API for device provisioning
 * 
 * Copyright (c) 2025 Arttu Tuominen. All rights reserved.
 * 
 * This software is licensed under commercial terms.
 * See LICENSE file for complete license terms.
 * 
 * NOTICE: This file contains proprietary information. Unauthorized 
 * distribution or use is strictly prohibited.
 */

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

/**
 * @brief Initialize the BLE manager.
 *
 * This function sets up the Bluetooth controller, initializes and enables Bluedroid,
 * registers the GAP and GATT callbacks, and starts the BLE GATT server.
 *
 * @return
 *      - ESP_OK on success
 *      - Appropriate error code from esp_err_t on failure
 */
esp_err_t ble_manager_init(void);

/**
 * @brief Task to handle confirmation from the mobile app that it received device info.
 *
 * This FreeRTOS task runs in the background, monitoring a flag that indicates whether
 * the device is waiting for confirmation. Based on success or failure, it handles
 * device reset or re-advertising over BLE.
 *
 * @param pvParameters Unused task parameter (pass NULL).
 */
void ble_manager_handle_device_info_confirmation(void* pvParameters);

#endif // BLE_MANAGER_H