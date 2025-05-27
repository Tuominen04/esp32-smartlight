#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <stdbool.h>
#include "esp_ota_ops.h"
#include "esp_http_server.h"
#include "esp_http_client.h"

// Configuration
#define OTA_BUF_SIZE 8192
#define MAX_HTTP_OUTPUT_BUFFER 2048

// Public API functions
esp_err_t ota_manager_init(void);
esp_err_t ota_manager_start_update(const char* firmware_url);
bool ota_manager_is_in_progress(void);
float ota_manager_get_progress(void);
const char* ota_manager_get_status_message(void);

// HTTP handlers (for registering with HTTP server)
esp_err_t ota_manager_firmware_info_handler(httpd_req_t *req);
esp_err_t ota_manager_ota_update_handler(httpd_req_t *req);  
esp_err_t ota_manager_progress_handler(httpd_req_t *req);

#endif // OTA_MANAGER_H