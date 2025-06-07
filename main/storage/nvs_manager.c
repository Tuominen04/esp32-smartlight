#include "nvs_manager.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

// other modules
#include "../common/common_defs.h"

static const char *NVS_TAG = "NVS_MANAGER";

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
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

/**
 * @brief Open NVS handle for read-only operations.
 *
 * Internal helper function to open the device namespace in read-only mode
 * for retrieving stored configuration data.
 *
 * @param[out] out_handle  Pointer to store the opened NVS handle.
 *
 * @return
 *      - ESP_OK on successful handle opening
 *      - ESP_ERR_* on NVS access failure
 */
static esp_err_t nvs_manager_read_nvs(nvs_handle_t *out_handle) {
    esp_err_t err = nvs_open(DEVICE_NAMESPACE, NVS_READONLY, out_handle);
    if (err != ESP_OK) {
        ESP_LOGE(NVS_TAG, "Error opening NVS: %s", esp_err_to_name(err));
    }
    return err;
}

/**
 * @brief Open NVS handle for read-write operations.
 *
 * Internal helper function to open the device namespace in read-write mode
 * for storing and modifying configuration data.
 *
 * @param[out] out_handle  Pointer to store the opened NVS handle.
 *
 * @return
 *      - ESP_OK on successful handle opening
 *      - ESP_ERR_* on NVS access failure
 */
static esp_err_t nvs_manager_read_write_nvs(nvs_handle_t *out_handle) {
    esp_err_t err = nvs_open(DEVICE_NAMESPACE, NVS_READWRITE, out_handle);
    if (err != ESP_OK) {
        ESP_LOGE(NVS_TAG, "Error opening NVS: %s", esp_err_to_name(err));
    }
    return err;
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
esp_err_t nvs_manager_get_device_info(char* out_device_name, size_t name_buf_size, char* out_device_id, size_t id_buf_size) {
    if (!out_device_name || !out_device_id) {
        ESP_LOGE(NVS_TAG, "Invalid output buffers: out_device_name=%p, out_device_id=%p", out_device_name, out_device_id);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (name_buf_size == 0 || id_buf_size == 0) {
        ESP_LOGE(NVS_TAG, "Invalid buffer sizes: name_buf_size=%d, id_buf_size=%d", name_buf_size, id_buf_size);
        return ESP_ERR_INVALID_ARG;
    }
    
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_manager_read_nvs(&nvs_handle);
    if (err != ESP_OK) return err;

    size_t actual_size;

    // Load device name
    actual_size = name_buf_size;
    err = nvs_get_str(nvs_handle, DEVICE_NAME_KEY, NULL, &actual_size);
    if (err == ESP_OK) {
        err = nvs_get_str(nvs_handle, DEVICE_NAME_KEY, out_device_name, &actual_size);
    } else if (err != ESP_OK) goto cleanup;

    // Load device ID
    actual_size = id_buf_size;
    err = nvs_get_str(nvs_handle, DEVICE_ID_KEY, NULL, &actual_size);
    if (err == ESP_OK) {
        err = nvs_get_str(nvs_handle, DEVICE_ID_KEY, out_device_id, &actual_size);
    } else if (err != ESP_OK) goto cleanup;

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
esp_err_t nvs_manager_get_wifi_credentials(char* out_ssid, size_t ssid_buf_size, char* out_password, size_t password_buf_size) {
    
    if (!out_ssid || !out_password) {
        ESP_LOGE(NVS_TAG, "Invalid output buffers: out_ssid=%p, out_password=%p", out_ssid, out_password);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (ssid_buf_size == 0 || password_buf_size == 0) {
        ESP_LOGE(NVS_TAG, "Invalid buffer sizes: ssid_buf_size=%d, password_buf_size=%d", ssid_buf_size, password_buf_size);
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_manager_read_nvs(&nvs_handle);
    if (err != ESP_OK) return err;

    size_t actual_size;

    // Load SSID
    actual_size = ssid_buf_size;
    err = nvs_get_str(nvs_handle, WIFI_SSID_KEY, NULL, &actual_size);
    if (err == ESP_OK) {
        err = nvs_get_str(nvs_handle, WIFI_SSID_KEY, out_ssid, &actual_size);
    } else if (err != ESP_OK) goto cleanup;

    // Load Password
    actual_size = password_buf_size;
    err = nvs_get_str(nvs_handle, WIFI_PASS_KEY, NULL, &actual_size);
    if (err == ESP_OK) {
        err = nvs_get_str(nvs_handle, WIFI_PASS_KEY, out_password, &actual_size);
    }

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
esp_err_t nvs_manager_delete_wifi_credentials() {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_manager_read_write_nvs(&nvs_handle);
    if (err != ESP_OK) return err;

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
esp_err_t nvs_manager_save_wifi_credentials(const char* ssid, const char* password) {
    if (!ssid || !password) {
        ESP_LOGE(NVS_TAG, "Invalid parameters: ssid=%p, password=%p", ssid, password);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (strlen(ssid) == 0 || strlen(password) == 0) {
        ESP_LOGE(NVS_TAG, "Empty SSID or password not allowed");
        return ESP_ERR_INVALID_ARG;
    }
    
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_manager_read_write_nvs(&nvs_handle);
    if (err != ESP_OK) return err;

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
esp_err_t nvs_manager_save_device_info(const char* device_id, const char* device_name) {
    if (!device_id || !device_name) {
        ESP_LOGE(NVS_TAG, "Invalid parameters: device_id=%p, device_name=%p", device_id, device_name);
        return ESP_ERR_INVALID_ARG;
    }
    
    if (strlen(device_id) == 0 || strlen(device_name) == 0) {
        ESP_LOGE(NVS_TAG, "Empty device_id or device_name not allowed");
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_manager_read_write_nvs(&nvs_handle);
    if (err != ESP_OK) return err;

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
