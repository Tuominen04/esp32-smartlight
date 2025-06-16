/**
 * @file nvs_manager.h
 * @brief NVS manager API for device and WiFi data
 * 
 * Copyright (c) 2025 Arttu Tuominen. All rights reserved.
 * 
 * This software is licensed under commercial terms.
 * See LICENSE file for complete license terms.
 * 
 * NOTICE: This file contains proprietary information. Unauthorized 
 * distribution or use is strictly prohibited.
 */

#ifndef NVS_MANAGER_H
#define NVS_MANAGER_H

#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

/**
 * @brief Initialize the NVS (Non-Volatile Storage) flash system.
 *
 * Initializes NVS flash and handles cases where flash needs to be erased
 * due to version incompatibilities or insufficient free pages.
 *
 * @return
 *      - ESP_OK on successful initialization
 *      - ESP_ERR_* on NVS initialization failure
 */
esp_err_t nvs_manager_init(void);

/**
 * @brief Retrieve stored device information from NVS.
 *
 * Loads the device name and device ID from non-volatile storage into
 * the provided buffers for application use.
 *
 * @param[out] out_device_name  Buffer to store the device name string.
 * @param[in]  name_buf_size    Size of the device name buffer.
 * @param[out] out_device_id    Buffer to store the device ID string.
 * @param[in]  id_buf_size      Size of the device ID buffer.
 *
 * @return
 *      - ESP_OK on successful data retrieval
 *      - ESP_ERR_NVS_NOT_FOUND if device info doesn't exist
 *      - ESP_ERR_* on NVS access or buffer size errors
 */
esp_err_t nvs_manager_get_device_info(char* out_device_name, size_t name_buf_size, char* out_device_id, size_t id_buf_size);

/**
 * @brief Retrieve stored WiFi credentials from NVS.
 *
 * Loads the WiFi SSID and password from non-volatile storage into
 * the provided buffers for network connection use.
 *
 * @param[out] out_ssid         Buffer to store the WiFi SSID string.
 * @param[in]  ssid_buf_size    Size of the SSID buffer.
 * @param[out] out_password     Buffer to store the WiFi password string.
 * @param[in]  password_buf_size Size of the password buffer.
 *
 * @return
 *      - ESP_OK on successful credential retrieval
 *      - ESP_ERR_NVS_NOT_FOUND if credentials don't exist
 *      - ESP_ERR_* on NVS access or buffer size errors
 */
esp_err_t nvs_manager_get_wifi_credentials(char* out_ssid, size_t ssid_buf_size, char* out_password, size_t password_buf_size);

/**
 * @brief Delete all stored WiFi credentials and device information from NVS.
 *
 * Removes WiFi credentials, device name, and device ID from non-volatile
 * storage, effectively resetting the device to unconfigured state.
 *
 * @return
 *      - ESP_OK on successful data deletion
 *      - ESP_ERR_* on NVS write or commit failure
 *
 * @note This function clears all device configuration data, requiring
 *       reconfiguration via BLE setup process.
 */
esp_err_t nvs_manager_delete_wifi_credentials();

/**
 * @brief Save WiFi credentials to NVS.
 *
 * Stores the provided WiFi SSID and password in non-volatile storage
 * for automatic connection on device restart.
 *
 * @param[in] ssid      WiFi network SSID to store.
 * @param[in] password  WiFi network password to store.
 *
 * @return
 *      - ESP_OK on successful credential storage
 *      - ESP_ERR_* on NVS write or commit failure
 */
esp_err_t nvs_manager_save_wifi_credentials(const char* ssid, const char* password);

/**
 * @brief Save device identification information to NVS.
 *
 * Stores the device ID and device name in non-volatile storage for
 * persistent device identification across reboots.
 *
 * @param[in] device_id    Unique device identifier string.
 * @param[in] device_name  Human-readable device name string.
 *
 * @return
 *      - ESP_OK on successful device info storage
 *      - ESP_ERR_* on NVS write or commit failure
 */
esp_err_t nvs_manager_save_device_info(const char* device_id, const char* device_name);

#endif // NVS_MANAGER_H
