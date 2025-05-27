#ifndef NVS_MANAGER_H
#define NVS_MANAGER_H

#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

esp_err_t nvs_manager_init(void);
esp_err_t nvs_manager_read_nvs(nvs_handle_t *out_handle);
esp_err_t nvs_manager_read_write_nvs(nvs_handle_t *out_handle);
esp_err_t nvs_manager_get_device_info(char* out_device_name, size_t name_buf_size, char* out_device_id, size_t id_buf_size);
esp_err_t nvs_manager_get_wifi_credentials(char* out_ssid, size_t ssid_buf_size, char* out_password, size_t password_buf_size);
esp_err_t nvs_manager_delete_wifi_credentials();
esp_err_t nvs_manager_save_wifi_credentials(const char* ssid, const char* password);
esp_err_t nvs_manager_save_device_info(const char* device_id, const char* device_name);

#endif // NVS_MANAGER_H