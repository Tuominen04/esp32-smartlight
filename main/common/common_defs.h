#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#include "esp_err.h"
#include <stdbool.h>
#include <stddef.h>

// Device configuration
#define DEVICE_NUMBER 1
#define DEVICE_NAME "ESP-C6-Light"
#define MAX_SSID_LEN 32
#define MAX_PASSWORD_LEN 64
#define MAX_DEVICE_NAME_LEN 32
#define MAX_DEVICE_ID_LEN 9

// NVS configuration
#define DEVICE_NAMESPACE "device_info"
#define WIFI_SSID_KEY "wifi_ssid"
#define WIFI_PASS_KEY "wifi_pass"
#define DEVICE_NAME_KEY "device_name"
#define DEVICE_ID_KEY "device_id"

// HTTP configuration
#define MAX_HTTP_OUTPUT_BUFFER 2048

// Task priorities
#define WIFI_TASK_PRIORITY 5
#define BLE_TASK_PRIORITY 5
#define OTA_TASK_PRIORITY 5

// Task stack sizes
#define WIFI_TASK_STACK_SIZE 4096
#define BLE_TASK_STACK_SIZE 4096
#define OTA_TASK_STACK_SIZE 16384

// Timeouts
#define WIFI_CONNECTION_TIMEOUT_MS 15000
#define HTTP_RESPONSE_TIMEOUT_MS 30000

// Error codes (application specific)
#define APP_ERR_INVALID_CREDENTIALS 0x1001
#define APP_ERR_WIFI_NOT_CONNECTED  0x1002
#define APP_ERR_BLE_NOT_INITIALIZED 0x1003

#endif // COMMON_DEFS_H

