/**
 * @file device_info.c
 * @brief Device information and BLE integration
 * 
 * Copyright (c) 2025 Arttu Tuominen. All rights reserved.
 * 
 * This software is licensed under commercial terms.
 * See LICENSE file for complete license terms.
 * 
 * NOTICE: This file contains proprietary information. Unauthorized 
 * distribution or use is strictly prohibited.
 */

#include "device_info.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include "cJSON.h"

// Include BLE headers that needed
#ifdef CONFIG_BT_ENABLED
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"
#else // BLE is not enabled - using mock implementations
#warning "BLE is not enabled - using mock BLE functions for testing"
#endif // CONFIG_BT_ENABLED

// Other moduls
#include "nvs_manager.h"

static const char *DEVICE_TAG = "DEVICE_HANDLING";
static const char *FIRMWARE_TAG = "FIRMWARE";

/** Handle for the BLE characteristic used to send device information. */
static uint16_t device_info_char_handle = 0;

/** BLE GATT server interface, set by BLE module when connected. */
static esp_gatt_if_t ble_gatts_if = ESP_GATT_IF_NONE;

/** BLE connection ID, set by BLE module when connected. */
static uint16_t ble_conn_id = 0;

/**
 * @brief Load stored device information from NVS.
 *
 * Attempts to load the device name and device ID from non-volatile storage.
 *
 * @param[out] out_device_name   Buffer to store the device name.
 * @param[in]  name_buf_size     Size of the device name buffer.
 * @param[out] out_device_id     Buffer to store the device ID.
 * @param[in]  id_buf_size       Size of the device ID buffer.
 * @return true if loading was successful, false otherwise.
 */
static bool load_device_info(char* out_device_name, size_t name_buf_size, char* out_device_id, size_t id_buf_size) {
    esp_err_t err = nvs_manager_get_device_info(out_device_name, name_buf_size, out_device_id, id_buf_size);

    if (err == ESP_OK) {
        ESP_LOGI(DEVICE_TAG, "Loaded device info - Name: %s, ID: %s", out_device_name, out_device_id);
        return true;
    } else {
        ESP_LOGI(DEVICE_TAG, "Failed to load device info: %s", esp_err_to_name(err));
        return false;
    }
}

/**
 * @brief Get information about the currently running firmware.
 *
 * Returns a dynamically allocated structure containing metadata such as
 * firmware version, project name, and compile time.
 *
 * @return Pointer to an `esp_app_desc_t` structure, or NULL on failure.
 *         Caller is responsible for freeing the memory.
 */
esp_app_desc_t* get_firmware_info(void) {
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t *app_desc = malloc(sizeof(esp_app_desc_t));
    
    if (app_desc == NULL) {
        ESP_LOGE(FIRMWARE_TAG, "Failed to allocate memory for app_desc");
        return NULL;
    }
    
    if (esp_ota_get_partition_description(running, app_desc) == ESP_OK) {
        ESP_LOGI(FIRMWARE_TAG, "Running firmware version: %s", app_desc->version);
        ESP_LOGI(FIRMWARE_TAG, "Running partition: %s", running->label);

        return app_desc;
    } else {
        ESP_LOGE(FIRMWARE_TAG, "Failed to get partition description");
        free(app_desc);
        return NULL;
    }
}


/**
 * @brief Save device information and WiFi credentials to NVS.
 *
 * This function creates a unique device name and ID using the MAC address.
 * If valid device info already exists in NVS, it skips saving again.
 *
 * @param ssid      WiFi SSID to save.
 * @param password  WiFi password to save.
 */
void device_manager_save_device_info(const char* ssid, const char* password) {
    if (!ssid || !password) {
        ESP_LOGE(DEVICE_TAG, "Invalid parameters: ssid=%p, password=%p", ssid, password);
        return;
    }
    
    if (strlen(ssid) == 0 || strlen(password) == 0) {
        ESP_LOGE(DEVICE_TAG, "Empty SSID or password not allowed");
        return;
    }
   
    // Check if already saved
    char existing_name[32] = {0};
    char existing_id[9] = {0};
    
    // If already saved and device name/ID is populated, don't overwrite
    if (load_device_info(existing_name, sizeof(existing_name), existing_id, sizeof(existing_id))) {
        if (strlen(existing_name) > 0 && strlen(existing_id) > 0) {
            ESP_LOGI(DEVICE_TAG, "Device info already exists in NVS: %s", existing_name);
            return;
        }
    }  

    esp_err_t err = nvs_manager_save_wifi_credentials(ssid, password);
    if (err != ESP_OK) {
        ESP_LOGI(DEVICE_TAG, "Failed to save WiFi credentials: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(DEVICE_TAG, "WiFi credentials save.");

    // Save device ID (using MAC address last 4 bytes)
    uint8_t mac[6];
    esp_err_t mac_err = esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
    if (mac_err != ESP_OK) {
    ESP_LOGE(DEVICE_TAG, "Failed to get MAC address: %s", esp_err_to_name(mac_err));
        return;
    }
    
    char device_id[9];
    sprintf(device_id, "%02X%02X%02X%02X", mac[2], mac[3], mac[4], mac[5]);
    
    // Save device name (ESP-C6-Light-XXXX)
    char device_name[32];
    sprintf(device_name, "ESP-C6-Light-%s", device_id);
    
    err = nvs_manager_save_device_info(device_id, device_name);
    if (err == ESP_OK) {
        ESP_LOGI(DEVICE_TAG, "Device info saved - Name: %s", device_name);
        return;
    } else {
        ESP_LOGI(DEVICE_TAG, "Failed to save device info: %s", esp_err_to_name(err));
        return;
    }
}

#ifdef CONFIG_BT_ENABLED
/**
 * @brief Send device information to the mobile app via BLE.
 *
 * Builds a JSON payload with device name, ID, IP address, and firmware version,
 * then updates the BLE characteristic and sends a notification if connected.
 */
void send_device_info_via_ble(void) {
    if (device_info_char_handle == 0 || ble_gatts_if == ESP_GATT_IF_NONE) {
        ESP_LOGE(DEVICE_TAG, "BLE not initialized or device info characteristic not created");
        return;
    }

    // Get device information
    char device_name[32] = {0};
    char device_id[9] = {0};
    char device_ip_str[16];
    char device_version[16] = {0};
    
    // Get IP address using the newer esp_netif API
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif == NULL) {
        ESP_LOGE(DEVICE_TAG, "Failed to get netif handle");
        return;
    }
    
    esp_netif_ip_info_t ip_info;
    esp_err_t res = esp_netif_get_ip_info(netif, &ip_info);
    if (res != ESP_OK) {
        ESP_LOGE(DEVICE_TAG, "Failed to get IP info: %s", esp_err_to_name(res));
        return;
    }
    
    load_device_info(device_name, sizeof(device_name), device_id, sizeof(device_id));
    sprintf(device_ip_str, IPSTR, IP2STR(&ip_info.ip));

    
    esp_app_desc_t *app_desc = get_firmware_info();
    if (app_desc != NULL) {
        strncpy(device_version, app_desc->version, sizeof(device_version) - 1);
        free(app_desc);
    } else {
        strcpy(device_version, "unknown");
    }

    // Create JSON with device info
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "name", device_name);
    cJSON_AddStringToObject(root, "id", device_id);
    cJSON_AddStringToObject(root, "ip", device_ip_str);
    cJSON_AddStringToObject(root, "version", device_version);
    
    char *json_str = cJSON_Print(root);
    ESP_LOGI(DEVICE_TAG, "Sending device info: %s", json_str);
    
    // Update the characteristic value
    esp_err_t ret = esp_ble_gatts_set_attr_value(device_info_char_handle, strlen(json_str), (uint8_t*)json_str);
    if (ret != ESP_OK) {
        ESP_LOGE(DEVICE_TAG, "Failed to set device info characteristic value, error = %x", ret);
    }
    
    // Send a notification if client is connected
    if (ble_conn_id != 0) {
        esp_ble_gatts_send_indicate(ble_gatts_if,
                                   ble_conn_id,
                                   device_info_char_handle,
                                   strlen(json_str),
                                   (uint8_t*)json_str,
                                   false);  // false means notification not indication
    }
    
    cJSON_Delete(root);
    free(json_str);
}

/**
 * @brief Set BLE GATT interface and connection ID.
 *
 * Called by the BLE module upon device connection to set up the necessary BLE context.
 *
 * @param gatts_if   BLE GATT interface.
 * @param conn_id    BLE connection ID.
 */
void device_info_set_ble_info(esp_gatt_if_t gatts_if, uint16_t conn_id) {
    ble_gatts_if = gatts_if;
    ble_conn_id = conn_id;
}

/**
 * @brief Set the BLE characteristic handle used to send device information.
 *
 * Called by the BLE module after the characteristic is created.
 *
 * @param handle  BLE GATT characteristic handle.
 */
void device_info_set_ble_handle(uint16_t handle) {
    device_info_char_handle = handle;
}
#else // CONFIG_BT_ENABLED is not defined
// Mock implementations when BLE is not enabled
static const char* MOCK_TAG = "DEVICE_INFO_MOCK";

void send_device_info_via_ble(void)
{
    ESP_LOGI(MOCK_TAG, "Mock: Sending device info via BLE");
}

void device_info_set_ble_info(esp_gatt_if_t gatts_if, uint16_t conn_id)
{
    ESP_LOGI(MOCK_TAG, "Mock: Setting BLE info (gatts_if=%d, conn_id=%d)", gatts_if, conn_id);
}

void device_info_set_ble_handle(uint16_t handle)
{
    ESP_LOGI(MOCK_TAG, "Mock: Setting BLE handle: %d", handle);
}
#endif
