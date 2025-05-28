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

    // Delete each key and check for errors
    err = nvs_set_str(nvs_handle, WIFI_SSID_KEY, NULL);
    if (err != ESP_OK) goto cleanup;

    err = nvs_set_str(nvs_handle, WIFI_PASS_KEY, NULL);
    if (err != ESP_OK) goto cleanup;

    err = nvs_set_str(nvs_handle, DEVICE_NAME_KEY, NULL);
    if (err != ESP_OK) goto cleanup;

    err = nvs_set_str(nvs_handle, DEVICE_ID_KEY, NULL);
    if (err != ESP_OK) goto cleanup;

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) goto cleanup;

cleanup:
    nvs_close(nvs_handle);
    return err;
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
