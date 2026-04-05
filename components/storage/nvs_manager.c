/**
 * @file nvs_manager.c
 * @brief NVS manager implementation for device and WiFi data
 * 
 * Copyright (c) 2025 Arttu Tuominen. All rights reserved.
 * 
 * This software is licensed under commercial terms.
 * See LICENSE file for complete license terms.
 * 
 * NOTICE: This file contains proprietary information. Unauthorized 
 * distribution or use is strictly prohibited.
 */

#include "nvs_manager.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

// other modules
#include "common_defs.h"

static const char *NVS_TAG = "NVS_MANAGER";

/**
 * @brief Open NVS namespace with requested access mode.
 *
 * @param[in] mode         NVS open mode.
 * @param[out] out_handle  Pointer where opened handle is stored.
 *
 * @return
 *      - ESP_OK on successful handle opening
 *      - ESP_ERR_INVALID_ARG when out_handle is NULL
 *      - ESP_ERR_* on NVS access failure
 */
static esp_err_t nvs_manager_open(nvs_open_mode_t mode, nvs_handle_t *out_handle)
{
  if (out_handle == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  esp_err_t err = nvs_open(DEVICE_NAMESPACE, mode, out_handle);
  if (err != ESP_OK) {
    ESP_LOGE(NVS_TAG, "Error opening NVS namespace: %s", esp_err_to_name(err));
  }
  return err;
}

/**
 * @brief Read an NVS string key into caller-provided buffer.
 *
 * @param[in] handle      Open NVS handle.
 * @param[in] key         NVS key to read.
 * @param[out] out_value  Destination buffer.
 * @param[in] out_size    Destination buffer size in bytes.
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG on invalid arguments
 *      - ESP_ERR_INVALID_SIZE when output buffer is too small
 *      - ESP_ERR_* returned by NVS APIs
 */
static esp_err_t nvs_manager_read_string(nvs_handle_t handle, const char *key, char *out_value, size_t out_size)
{
  if (key == NULL || out_value == NULL || out_size == 0U) {
    return ESP_ERR_INVALID_ARG;
  }

  size_t required_size = 0U;
  esp_err_t err = nvs_get_str(handle, key, NULL, &required_size);
  if (err != ESP_OK) {
    return err;
  }

  if (required_size > out_size) {
    ESP_LOGE(NVS_TAG, "Buffer too small for key %s (required=%u, provided=%u)",
      key,
      (unsigned int)required_size,
      (unsigned int)out_size);
    return ESP_ERR_INVALID_SIZE;
  }

  return nvs_get_str(handle, key, out_value, &required_size);
}

/**
 * @brief Validate text input length against non-empty and max size constraints.
 *
 * @param[in] value       Input string to validate.
 * @param[in] max_len     Maximum allowed length excluding null terminator.
 * @param[in] field_name  Field name used for logging.
 *
 * @return
 *      - ESP_OK when valid
 *      - ESP_ERR_INVALID_ARG when invalid
 */
static esp_err_t nvs_manager_validate_text(const char *value, size_t max_len, const char *field_name)
{
  if (value == NULL) {
    ESP_LOGE(NVS_TAG, "Invalid parameter: %s is NULL", field_name);
    return ESP_ERR_INVALID_ARG;
  }

  size_t len = strnlen(value, max_len + 1U);
  if (len == 0U || len > max_len) {
    ESP_LOGE(NVS_TAG, "Invalid %s length", field_name);
    return ESP_ERR_INVALID_ARG;
  }

  return ESP_OK;
}

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
esp_err_t nvs_manager_init(void)
{
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_LOGW(NVS_TAG, "NVS requires erase due to state: %s", esp_err_to_name(ret));
    ret = nvs_flash_erase();
    if (ret != ESP_OK) {
      ESP_LOGE(NVS_TAG, "Failed to erase NVS flash: %s", esp_err_to_name(ret));
      return ret;
    }
    ret = nvs_flash_init();
  }
  if (ret != ESP_OK) {
    ESP_LOGE(NVS_TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
  }
  return ret;
}

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
esp_err_t nvs_manager_get_device_info(char *out_device_name, size_t name_buf_size, char *out_device_id, size_t id_buf_size)
{
  if (!out_device_name || !out_device_id) {
    ESP_LOGE(NVS_TAG, "Invalid output buffers for device info retrieval");
    return ESP_ERR_INVALID_ARG;
  }

  if (name_buf_size < MAX_DEVICE_NAME_LEN || id_buf_size < MAX_DEVICE_ID_LEN) {
    ESP_LOGE(NVS_TAG, "Device info output buffers are too small");
    return ESP_ERR_INVALID_ARG;
  }

  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_manager_open(NVS_READONLY, &nvs_handle);
  if (err != ESP_OK) {
    return err;
  }

  // Load device name
  err = nvs_manager_read_string(nvs_handle, DEVICE_NAME_KEY, out_device_name, name_buf_size);
  if (err != ESP_OK) {
    goto cleanup;
  }

  // Load device ID
  err = nvs_manager_read_string(nvs_handle, DEVICE_ID_KEY, out_device_id, id_buf_size);

cleanup:
  nvs_close(nvs_handle);
  return err;
}

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
esp_err_t nvs_manager_get_wifi_credentials(char *out_ssid, size_t ssid_buf_size, char *out_password, size_t password_buf_size)
{
  if (!out_ssid || !out_password) {
    ESP_LOGE(NVS_TAG, "Invalid output buffers for WiFi credential retrieval");
    return ESP_ERR_INVALID_ARG;
  }

  if (ssid_buf_size < MAX_SSID_BUF_LEN || password_buf_size < MAX_PASSWORD_LEN) {
    ESP_LOGE(NVS_TAG, "WiFi output buffers are too small");
    return ESP_ERR_INVALID_ARG;
  }

  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_manager_open(NVS_READONLY, &nvs_handle);
  if (err != ESP_OK) {
    return err;
  }

  // Load SSID
  err = nvs_manager_read_string(nvs_handle, WIFI_SSID_KEY, out_ssid, ssid_buf_size);
  if (err != ESP_OK) {
    goto cleanup;
  }

  // Load Password
  err = nvs_manager_read_string(nvs_handle, WIFI_PASS_KEY, out_password, password_buf_size);

cleanup:
  nvs_close(nvs_handle);
  return err;
}

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
esp_err_t nvs_manager_delete_wifi_credentials(void)
{
  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_manager_open(NVS_READWRITE, &nvs_handle);
  if (err != ESP_OK) {
    return err;
  }

  // Use nvs_erase_key instead of nvs_set_str with NULL
  esp_err_t wifi_ssid_err = nvs_erase_key(nvs_handle, WIFI_SSID_KEY);
  esp_err_t wifi_pass_err = nvs_erase_key(nvs_handle, WIFI_PASS_KEY);
  esp_err_t device_name_err = nvs_erase_key(nvs_handle, DEVICE_NAME_KEY);
  esp_err_t device_id_err = nvs_erase_key(nvs_handle, DEVICE_ID_KEY);

  // Log individual results (ESP_ERR_NVS_NOT_FOUND is OK - means key didn't exist)
  if (wifi_ssid_err != ESP_OK && wifi_ssid_err != ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGW(NVS_TAG, "Failed to erase WIFI_SSID_KEY: %s", esp_err_to_name(wifi_ssid_err));
  }
  if (wifi_pass_err != ESP_OK && wifi_pass_err != ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGW(NVS_TAG, "Failed to erase WIFI_PASS_KEY: %s", esp_err_to_name(wifi_pass_err));
  }
  if (device_name_err != ESP_OK && device_name_err != ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGW(NVS_TAG, "Failed to erase DEVICE_NAME_KEY: %s", esp_err_to_name(device_name_err));
  }
  if (device_id_err != ESP_OK && device_id_err != ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGW(NVS_TAG, "Failed to erase DEVICE_ID_KEY: %s", esp_err_to_name(device_id_err));
  }

  // Commit the changes
  err = nvs_commit(nvs_handle);
  if (err != ESP_OK) {
    ESP_LOGE(NVS_TAG, "Error committing NVS after delete: %s", esp_err_to_name(err));
  }

  nvs_close(nvs_handle);
  return err;  // Return the commit result
}

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
esp_err_t nvs_manager_save_wifi_credentials(const char *ssid, const char *password)
{
  if (nvs_manager_validate_text(ssid, MAX_SSID_BUF_LEN - 1U, "ssid") != ESP_OK) {
    return ESP_ERR_INVALID_ARG;
  }
  if (nvs_manager_validate_text(password, MAX_PASSWORD_LEN - 1U, "password") != ESP_OK) {
    return ESP_ERR_INVALID_ARG;
  }

  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_manager_open(NVS_READWRITE, &nvs_handle);
  if (err != ESP_OK) {
    return err;
  }

  err = nvs_set_str(nvs_handle, WIFI_SSID_KEY, ssid);
  if (err != ESP_OK) goto cleanup;

  err = nvs_set_str(nvs_handle, WIFI_PASS_KEY, password);
  if (err != ESP_OK) goto cleanup;

  err = nvs_commit(nvs_handle);
  if (err != ESP_OK) {
    ESP_LOGE(NVS_TAG, "Error committing NVS: %s", esp_err_to_name(err));
    goto cleanup;
  }
cleanup:
  nvs_close(nvs_handle);
  return err;
}

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
esp_err_t nvs_manager_save_device_info(const char *device_id, const char *device_name)
{
  if (nvs_manager_validate_text(device_id, MAX_DEVICE_ID_LEN - 1U, "device_id") != ESP_OK) {
    return ESP_ERR_INVALID_ARG;
  }
  if (nvs_manager_validate_text(device_name, MAX_DEVICE_NAME_LEN - 1U, "device_name") != ESP_OK) {
    return ESP_ERR_INVALID_ARG;
  }

  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_manager_open(NVS_READWRITE, &nvs_handle);
  if (err != ESP_OK) {
    return err;
  }

  err = nvs_set_str(nvs_handle, DEVICE_ID_KEY, device_id);
  if (err != ESP_OK) goto cleanup;

  err = nvs_set_str(nvs_handle, DEVICE_NAME_KEY, device_name);
  if (err != ESP_OK) goto cleanup;

  err = nvs_commit(nvs_handle);
  if (err != ESP_OK) {
    ESP_LOGE(NVS_TAG, "Error committing NVS: %s", esp_err_to_name(err));
    goto cleanup;
  }
cleanup:
  nvs_close(nvs_handle);
  return err;
}
