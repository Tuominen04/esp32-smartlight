/**
 * @file common_defs.h
 * @brief Common definitions and constants used throughout the ESP light client application.
 *
 * This header contains shared configuration values, buffer sizes, task parameters,
 * and application-specific error codes used across multiple modules.
 */
#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#include "esp_err.h"
#include <stdbool.h>
#include <stddef.h>

/* === Device configuration === */
/* ============================ */
/** Unique device instance number for identification. */
#define DEVICE_NUMBER 1

/** Default device name broadcasted via BLE and used in HTTP responses. */
#define DEVICE_NAME "ESP-C6-Light"

/** Maximum length for WiFi SSID strings including null terminator. */
#define MAX_SSID_LEN 32

/** Maximum length for WiFi password strings including null terminator. */
#define MAX_PASSWORD_LEN 64

/** Maximum length for device name strings including null terminator. */
#define MAX_DEVICE_NAME_LEN 32

/** Maximum length for device ID strings including null terminator. */
#define MAX_DEVICE_ID_LEN 9

/* === NVS configuration === */
/* ========================= */
/** NVS namespace name for storing device-related data. */
#define DEVICE_NAMESPACE "device_info"

/** NVS key for storing WiFi SSID. */
#define WIFI_SSID_KEY "wifi_ssid"

/** NVS key for storing WiFi password. */
#define WIFI_PASS_KEY "wifi_pass"

/** NVS key for storing device name. */
#define DEVICE_NAME_KEY "device_name"

/** NVS key for storing device ID. */
#define DEVICE_ID_KEY "device_id"

/* === HTTP configuration === */
/* ========================== */
/** Maximum size for HTTP response buffers in bytes. */
#define MAX_HTTP_OUTPUT_BUFFER 2048

/* === Task priorities === */
/* ======================= */
/** FreeRTOS priority level for WiFi management tasks. */
#define WIFI_TASK_PRIORITY 5

/** FreeRTOS priority level for BLE management tasks. */
#define BLE_TASK_PRIORITY 5

/** FreeRTOS priority level for OTA update tasks. */
#define OTA_TASK_PRIORITY 5

/* === Task stack sizes === */
/* ======================== */
/** Stack size in bytes for WiFi management tasks. */
#define WIFI_TASK_STACK_SIZE 4096

/** Stack size in bytes for BLE management tasks. */
#define BLE_TASK_STACK_SIZE 4096

/** Stack size in bytes for OTA update tasks (larger due to HTTP operations). */
#define OTA_TASK_STACK_SIZE 16384

/* === Timeouts === */
/* ================ */
/** Maximum time to wait for WiFi connection in milliseconds. */
#define WIFI_CONNECTION_TIMEOUT_MS 15000

/** Maximum time to wait for HTTP responses in milliseconds. */
#define HTTP_RESPONSE_TIMEOUT_MS 30000

// Error codes (application specific)
/** Application error code for invalid WiFi credentials. */
#define APP_ERR_INVALID_CREDENTIALS 0x1001

/** Application error code for WiFi not connected state. */
#define APP_ERR_WIFI_NOT_CONNECTED  0x1002

/** Application error code for BLE not initialized state. */
#define APP_ERR_BLE_NOT_INITIALIZED 0x1003

#endif // COMMON_DEFS_H

