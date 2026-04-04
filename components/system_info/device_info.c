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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
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
#endif // CONFIG_BT_ENABLED

// Other moduls
#include "nvs_manager.h"

static const char *DEVICE_TAG = "DEVICE_HANDLING";
static const char *FIRMWARE_TAG = "FIRMWARE";

#ifdef CONFIG_BT_ENABLED
/** Handle for the BLE characteristic used to send device information. */
static uint16_t device_info_char_handle = 0;

/** BLE GATT server interface, set by BLE module when connected. */
static esp_gatt_if_t ble_gatts_if = ESP_GATT_IF_NONE;

/** BLE connection ID, set by BLE module when connected. */
static uint16_t ble_conn_id = 0;
#endif

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
static bool load_device_info(char *out_device_name, size_t name_buf_size, char *out_device_id, size_t id_buf_size)
{
  esp_err_t err = nvs_manager_get_device_info(out_device_name, name_buf_size, out_device_id, id_buf_size);

  if (err == ESP_OK) {
    ESP_LOGI(DEVICE_TAG, "Loaded device info from NVS");
    return true;
  }

  ESP_LOGI(DEVICE_TAG, "Failed to load device info: %s", esp_err_to_name(err));
  return false;
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
esp_app_desc_t *get_firmware_info(void)
{
  const esp_partition_t *running = esp_ota_get_running_partition();
  if (running == NULL) {
    ESP_LOGE(FIRMWARE_TAG, "Failed to get running partition");
    return NULL;
  }

  esp_app_desc_t *app_desc = calloc(1, sizeof(esp_app_desc_t));
  if (app_desc == NULL) {
    ESP_LOGE(FIRMWARE_TAG, "Failed to allocate memory for app_desc");
    return NULL;
  }

  if (esp_ota_get_partition_description(running, app_desc) != ESP_OK) {
    ESP_LOGE(FIRMWARE_TAG, "Failed to get partition description");
    free(app_desc);
    return NULL;
  }

  ESP_LOGI(FIRMWARE_TAG, "Running firmware version: %s", app_desc->version);
  ESP_LOGI(FIRMWARE_TAG, "Running partition: %s", running->label);
  return app_desc;
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
void device_manager_save_device_info(const char *ssid, const char *password)
{
  if (!ssid || !password) {
    ESP_LOGE(DEVICE_TAG, "Invalid parameters for saving device info");
    return;
  }

  if (strlen(ssid) == 0 || strlen(password) == 0) {
    ESP_LOGE(DEVICE_TAG, "Empty SSID or password not allowed");
    return;
  }

  char existing_name[MAX_DEVICE_NAME_LEN] = {0};
  char existing_id[MAX_DEVICE_ID_LEN] = {0};
  if (load_device_info(existing_name, sizeof(existing_name), existing_id, sizeof(existing_id)) &&
        strlen(existing_name) > 0 && strlen(existing_id) > 0) {
    ESP_LOGI(DEVICE_TAG, "Device info already exists in NVS");
    return;
  }

  esp_err_t err = nvs_manager_save_wifi_credentials(ssid, password);
  if (err != ESP_OK) {
    ESP_LOGE(DEVICE_TAG, "Failed to save WiFi credentials: %s", esp_err_to_name(err));
    return;
  }

  uint8_t mac[6] = {0};
  err = esp_wifi_get_mac(WIFI_IF_STA, mac);
  if (err != ESP_OK) {
    ESP_LOGE(DEVICE_TAG, "Failed to get MAC address: %s", esp_err_to_name(err));
    return;
  }

  char device_id[MAX_DEVICE_ID_LEN] = {0};
  int written = snprintf(device_id, sizeof(device_id), "%02X%02X%02X%02X", mac[2], mac[3], mac[4], mac[5]);
  if (written <= 0 || written >= (int)sizeof(device_id)) {
    ESP_LOGE(DEVICE_TAG, "Failed to format device ID");
    return;
  }

  char device_name[MAX_DEVICE_NAME_LEN] = {0};
  written = snprintf(device_name, sizeof(device_name), "%s-%s", DEVICE_NAME, device_id);
  if (written <= 0 || written >= (int)sizeof(device_name)) {
    ESP_LOGE(DEVICE_TAG, "Failed to format device name");
    return;
  }

  err = nvs_manager_save_device_info(device_id, device_name);
  if (err != ESP_OK) {
    ESP_LOGE(DEVICE_TAG, "Failed to save device info: %s", esp_err_to_name(err));
    return;
  }

  ESP_LOGI(DEVICE_TAG, "Device info saved successfully");
}

#ifdef CONFIG_BT_ENABLED
/**
 * @brief Send device information to the mobile app via BLE.
 *
 * Builds a JSON payload with device name, ID, IP address, and firmware version,
 * then updates the BLE characteristic and sends a notification if connected.
 */
void send_device_info_via_ble(void)
{
  ESP_LOGI(DEVICE_TAG, "Attempting to send device info via BLE");

  if (device_info_char_handle == 0) {
    ESP_LOGE(DEVICE_TAG, "Device info characteristic not created");
    return;
  }

  if (ble_gatts_if == ESP_GATT_IF_NONE) {
    ESP_LOGE(DEVICE_TAG, "BLE GATT interface not initialized");
    return;
  }

  // Get device information
  char device_name[MAX_DEVICE_NAME_LEN] = {0};
  char device_id[MAX_DEVICE_ID_LEN] = {0};
  char device_ip_str[16] = {0};
  char device_version[16] = {0};

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
  snprintf(device_ip_str, sizeof(device_ip_str), IPSTR, IP2STR(&ip_info.ip));

  esp_app_desc_t *app_desc = get_firmware_info();
  if (app_desc != NULL) {
    snprintf(device_version, sizeof(device_version), "%s", app_desc->version);
    free(app_desc);
  } else {
    snprintf(device_version, sizeof(device_version), "%s", "unknown");
  }

  // Create JSON with device info
  cJSON *root = cJSON_CreateObject();
  if (root == NULL) {
    ESP_LOGE(DEVICE_TAG, "Failed to create JSON object for device info");
    return;
  }

  cJSON_AddStringToObject(root, "name", device_name);
  cJSON_AddStringToObject(root, "id", device_id);
  cJSON_AddStringToObject(root, "ip", device_ip_str);
  cJSON_AddStringToObject(root, "version", device_version);

  char *json_str = cJSON_PrintUnformatted(root);
  if (json_str == NULL) {
    ESP_LOGE(DEVICE_TAG, "Failed to serialize device info JSON");
    cJSON_Delete(root);
    return;
  }

  size_t json_len = strlen(json_str);
  if (json_len == 0U || json_len > UINT16_MAX) {
    ESP_LOGE(DEVICE_TAG, "Invalid JSON payload size for BLE notify");
    cJSON_Delete(root);
    free(json_str);
    return;
  }

  esp_err_t ret = esp_ble_gatts_set_attr_value(device_info_char_handle, (uint16_t)json_len, (uint8_t *)json_str);
  if (ret != ESP_OK) {
    ESP_LOGE(DEVICE_TAG, "Failed to set device info characteristic value: %s", esp_err_to_name(ret));
    cJSON_Delete(root);
    free(json_str);
    return;
  }

  vTaskDelay(pdMS_TO_TICKS(100));

  ret = esp_ble_gatts_send_indicate(
        ble_gatts_if,
        ble_conn_id,
        device_info_char_handle,
        (uint16_t)json_len,
        (uint8_t *)json_str,
        false);

  if (ret != ESP_OK) {
    ESP_LOGE(DEVICE_TAG, "Failed to send device info notification: %s", esp_err_to_name(ret));
  } else {
    ESP_LOGI(DEVICE_TAG, "Device info notification sent successfully");
  }

  vTaskDelay(pdMS_TO_TICKS(100));
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
void device_info_set_ble_info(esp_gatt_if_t gatts_if, uint16_t conn_id)
{
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
void device_info_set_ble_handle(uint16_t handle)
{
  device_info_char_handle = handle;
}
#endif
