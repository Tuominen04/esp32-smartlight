/**
 * @file ota_manager.h
 * @brief OTA manager API for firmware updates
 * 
 * Copyright (c) 2025 Arttu Tuominen. All rights reserved.
 * 
 * This software is licensed under commercial terms.
 * See LICENSE file for complete license terms.
 * 
 * NOTICE: This file contains proprietary information. Unauthorized 
 * distribution or use is strictly prohibited.
 */

#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <stdbool.h>
#include "esp_ota_ops.h"
#include "esp_http_server.h"
#include "esp_http_client.h"

#define OTA_BUF_SIZE 8192
#define MAX_HTTP_OUTPUT_BUFFER 2048

/**
 * @brief Initialize the OTA manager module.
 *
 * Performs basic initialization of the OTA management system.
 * Currently just logs initialization status.
 *
 * @return ESP_OK always.
 */
esp_err_t ota_manager_init(void);

/**
 * @brief Start an OTA update programmatically.
 *
 * Alternative to HTTP handler for starting OTA updates from application code.
 * Creates a background task to handle the update process.
 *
 * @param[in] firmware_url  URL of the firmware binary to download.
 *
 * @return
 *      - ESP_OK on successful task creation
 *      - ESP_ERR_INVALID_ARG if URL is NULL
 *      - ESP_ERR_INVALID_STATE if OTA already in progress
 *      - ESP_ERR_NO_MEM on memory allocation failure
 *      - ESP_FAIL on task creation failure
 */
esp_err_t ota_manager_start_update(const char* firmware_url);

/**
 * @brief Check if an OTA update is currently in progress.
 *
 * @return
 *      - true if OTA update is active
 *      - false if no OTA operation is running
 */
bool ota_manager_is_in_progress(void);

/**
 * @brief Get the current OTA progress percentage.
 *
 * @return Progress value from 0.0 to 100.0 indicating download completion.
 */
float ota_manager_get_progress(void);

/**
 * @brief Get the current OTA status message.
 *
 * @return Pointer to string describing the current OTA operation stage.
 */
const char* ota_manager_get_status_message(void);

/* === HTTP handlers (for registering with HTTP server) === */

/**
 * @brief HTTP handler for firmware information requests.
 *
 * Returns current firmware details including version, project name,
 * SHA256 hash, build date/time, and OTA status in JSON format.
 *
 * @param[in] req  HTTP request structure for the GET /ota/firmware-info endpoint.
 *
 * @return
 *      - ESP_OK on successful response transmission
 *      - ESP_FAIL on memory allocation or partition access failure
 */
esp_err_t ota_manager_firmware_info_handler(httpd_req_t *req);

/**
 * @brief HTTP handler for initiating OTA updates.
 *
 * Processes POST requests to start OTA updates, parsing the firmware URL
 * from JSON payload and launching the update task asynchronously.
 *
 * @param[in] req  HTTP request structure for the POST /ota/update endpoint.
 *
 * @return
 *      - ESP_OK on successful task creation and response
 *      - ESP_FAIL on invalid request, URL parsing, or task creation failure
 *
 * @note Returns immediately after starting the task; actual update runs
 *       in background. Only one OTA process can run at a time.
 */
esp_err_t ota_manager_ota_update_handler(httpd_req_t *req);

/**
 * @brief HTTP handler for OTA progress monitoring.
 *
 * Returns current OTA status, progress percentage, and status message
 * in JSON format. Implements rate limiting to prevent excessive requests.
 *
 * @param[in] req  HTTP request structure for the GET /ota/progress endpoint.
 *
 * @return ESP_OK always (rate limited requests get 503 status).
 *
 * @note Rate limited to one request per 500ms during active OTA to
 *       prevent overwhelming the system.
 */
esp_err_t ota_manager_progress_handler(httpd_req_t *req);

#endif // OTA_MANAGER_H