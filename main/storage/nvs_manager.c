#include "nvs_manager.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

// other modules
#include "../common/common_defs.h"

static const char *NVS_TAG = "NVS_MANAGER";

esp_err_t nvs_manager_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

esp_err_t nvs_manager_read_nvs(nvs_handle_t *out_handle) {
    esp_err_t err = nvs_open(DEVICE_NAMESPACE, NVS_READONLY, out_handle);
    if (err != ESP_OK) {
        ESP_LOGE(NVS_TAG, "Error opening NVS: %s", esp_err_to_name(err));
    }
    return err;
}

esp_err_t nvs_manager_read_write_nvs(nvs_handle_t *out_handle) {
    esp_err_t err = nvs_open(DEVICE_NAMESPACE, NVS_READWRITE, out_handle);
    if (err != ESP_OK) {
        ESP_LOGE(NVS_TAG, "Error opening NVS: %s", esp_err_to_name(err));
    }
    return err;
}

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

