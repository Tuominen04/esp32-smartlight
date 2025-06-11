/**
 * @file ota_manager.c
 * @brief OTA manager implementation for firmware updates
 * 
 * Copyright (c) 2025 Arttu Tuominen. All rights reserved.
 * 
 * This software is licensed under commercial terms.
 * See LICENSE file for complete license terms.
 * 
 * NOTICE: This file contains proprietary information. Unauthorized 
 * distribution or use is strictly prohibited.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "cJSON.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_app_format.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#include "ota_manager.h"

static const char *OTA_TAG = "OTA";
static const char *OTA_HTTP_TAG = "OTA-HTTP";
static const char *OTA_TASK_TAG = "OTA-TASK";
static const char *OTA_PROGRESS_TAG = "OTA-PROGRESS";
static const char *OTA_HANDLER_TAG = "OTA-HANDLER";

/** Flag indicating whether an OTA update is currently in progress. */
static bool ota_in_progress = false;

/** Current progress percentage of the OTA update (0.0 to 100.0). */
static float ota_progress_percentage = 0.0;

/** Current status message describing the OTA update stage. */
static char ota_status_message[64] = "Initializing...";

/** Timestamp of the last progress request to implement rate limiting. */
static int64_t last_progress_request = 0;

/**
 * @brief HTTP event handler for OTA update operations.
 *
 * Handles various HTTP client events during OTA firmware download,
 * providing logging for connection status, data transfer, and errors.
 *
 * @param[in] evt  HTTP client event structure containing event data.
 *
 * @return ESP_OK always (events are logged but don't stop the process).
 */
static esp_err_t ota_manager_http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGE(OTA_HTTP_TAG, "Error occurred");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(OTA_HTTP_TAG, "Connected to server");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(OTA_HTTP_TAG, "Headers sent");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(OTA_HTTP_TAG, "Received header - %s: %s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(OTA_HTTP_TAG, "Received data chunk - %d bytes", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(OTA_HTTP_TAG, "Connection finished");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(OTA_HTTP_TAG, "Disconnected from server");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGI(OTA_HTTP_TAG, "Redirect received");
        break;
    }
    return ESP_OK;
}

/**
 * @brief Perform the actual OTA firmware update process.
 *
 * Downloads firmware from the specified URL, validates the image,
 * writes it to the update partition, and sets it as the boot partition.
 * Updates progress indicators throughout the process.
 *
 * @param[in] firmware_url  URL of the firmware binary to download.
 *
 * @return
 *      - true on successful update (device will reboot)
 *      - false on any failure during the update process
 *
 * @note This function blocks until completion and will restart the device
 *       on successful update. Progress is updated via global variables.
 */
static bool ota_manager_perform_ota_update(const char* firmware_url) 
{        
    ESP_LOGI(OTA_TAG, "Starting OTA update process from URL: %s", firmware_url);

    // Reset progress indicators
    ota_progress_percentage = 0.0;
    strcpy(ota_status_message, "Connecting to server...");

    esp_http_client_config_t config = {
        .url = firmware_url,
        .event_handler = ota_manager_http_event_handler,
        .buffer_size = OTA_BUF_SIZE,
        .timeout_ms = 30000,  // 30 seconds timeout
        .skip_cert_common_name_check = true,
        .cert_pem = NULL          // No certificate 
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(OTA_TAG, "Failed to initialize HTTP client");
        strcpy(ota_status_message, "Failed to initialize HTTP client");
        return false;
    }

    // Open connection
    strcpy(ota_status_message, "Opening HTTP connection...");
    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(OTA_TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        strcpy(ota_status_message, "Failed to open HTTP connection");
        esp_http_client_cleanup(client);
        return false;
    }

    // Get content length
    strcpy(ota_status_message, "Fetching firmware size...");
    int content_length = esp_http_client_fetch_headers(client);
    if (content_length < 0) {
        ESP_LOGE(OTA_TAG, "Failed to get firmware size");
        strcpy(ota_status_message, "Failed to get firmware size");
        esp_http_client_cleanup(client);
        return false;
    }

    ESP_LOGI(OTA_TAG, "Firmware size: %d bytes", content_length);
    sprintf(ota_status_message, "Firmware size: %d bytes", content_length);

    // Check firmware size
    if (content_length > 0) {
        // Get OTA handle
        esp_ota_handle_t ota_handle = 0;
        const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);

        if (update_partition == NULL) {
            ESP_LOGE(OTA_TAG, "Failed to get update partition");
            strcpy(ota_status_message, "Failed to get update partition");
            esp_http_client_cleanup(client);
            return false;
        }
        
        ESP_LOGI(OTA_TAG, "Writing to partition label: %s, subtype: %d, offset: 0x%lu",
                 update_partition->label, update_partition->subtype, update_partition->address);
        sprintf(ota_status_message, "Writing to partition: %s", update_partition->label);

        // Begin OTA
        err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &ota_handle);
        if (err != ESP_OK) {
            ESP_LOGE(OTA_TAG, "Failed to begin OTA update: %s", esp_err_to_name(err));
            strcpy(ota_status_message, "Failed to begin OTA update");
            esp_http_client_cleanup(client);
            return false;
        }

        // Create buffer for data
        char *upgrade_data_buf = (char *)malloc(4096);
        if (!upgrade_data_buf) {
            ESP_LOGE(OTA_TAG, "Failed to allocate memory for upgrade data buffer");
            strcpy(ota_status_message, "Failed to allocate memory");
            esp_ota_end(ota_handle);
            esp_http_client_cleanup(client);
            return false;
        }

        // Read data and write to flash
        int binary_file_length = 0;
        int data_read;
        bool image_header_checked = false;

        while (1) {
            data_read = esp_http_client_read(client, upgrade_data_buf, 4096);
            if (data_read < 0) {
                ESP_LOGE(OTA_TAG, "Error reading data");
                strcpy(ota_status_message, "Error reading data");
                free(upgrade_data_buf);
                esp_ota_end(ota_handle);
                esp_http_client_cleanup(client);
                return false;
            } else if (data_read == 0) {
                // Download completed
                break;
            }

            // Check image header first
            if (!image_header_checked) {
                esp_app_desc_t new_app_info;
                if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
                    // Check current running version with the one we're trying to download
                    memcpy(&new_app_info, &upgrade_data_buf[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
                    ESP_LOGI(OTA_TAG, "New firmware version: %s", new_app_info.version);
                    
                    // Get info about running firmware
                    const esp_partition_t *running = esp_ota_get_running_partition();
                    esp_app_desc_t running_app_info;
                    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
                        ESP_LOGI(OTA_TAG, "Running firmware version: %s", running_app_info.version);
                        sprintf(ota_status_message, "Downloading...");
                    }
                    
                    image_header_checked = true;
                } else {
                    ESP_LOGE(OTA_TAG, "Invalid image header");
                    strcpy(ota_status_message, "Invalid image header");
                    free(upgrade_data_buf);
                    esp_ota_end(ota_handle);
                    esp_http_client_cleanup(client);
                    return false;
                }
            }

            // Write data to flash
            err = esp_ota_write(ota_handle, (const void *)upgrade_data_buf, data_read);
            if (err != ESP_OK) {
                ESP_LOGE(OTA_TAG, "Error writing OTA data: %s", esp_err_to_name(err));
                strcpy(ota_status_message, "Error writing OTA data");
                free(upgrade_data_buf);
                esp_ota_end(ota_handle);
                esp_http_client_cleanup(client);
                return false;
            }

            binary_file_length += data_read;
            // Update progress percentage (0-100)
            ota_progress_percentage = ((float)binary_file_length / content_length) * 100;
            
            ESP_LOGI(OTA_TAG, "Downloaded %d/%d bytes (%.1f%%)", 
                binary_file_length, content_length, ota_progress_percentage);
            // Update status message
            sprintf(ota_status_message, "Downloading...");
        }

        // Clean up
        free(upgrade_data_buf);
        
        // Check download completed
        if (binary_file_length != content_length) {
            ESP_LOGE(OTA_TAG, "Download incomplete: %d/%d bytes", binary_file_length, content_length);
            strcpy(ota_status_message, "Download incomplete");
            esp_ota_end(ota_handle);
            esp_http_client_cleanup(client);
            return false;
        }

        // End OTA and validate
        strcpy(ota_status_message, "Validating firmware...");
        err = esp_ota_end(ota_handle);
        if (err != ESP_OK) {
            if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
                ESP_LOGE(OTA_TAG, "Firmware validation failed");
                strcpy(ota_status_message, "Firmware validation failed");
            } else {
                ESP_LOGE(OTA_TAG, "Error finalizing OTA update: %s", esp_err_to_name(err));
                strcpy(ota_status_message, "Error finalizing OTA update");
            }
            esp_http_client_cleanup(client);
            return false;
        }
        // Set boot partition
        strcpy(ota_status_message, "Setting boot partition...");
        err = esp_ota_set_boot_partition(update_partition);
        if (err != ESP_OK) {
            ESP_LOGE(OTA_TAG, "Failed to set boot partition: %s", esp_err_to_name(err));
            strcpy(ota_status_message, "Failed to set boot partition");
            esp_http_client_cleanup(client);
            return false;
        }

        ESP_LOGI(OTA_TAG, "Update successful! Rebooting in 5 seconds...");
        strcpy(ota_status_message, "Update successful! Rebooting...");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        esp_restart();
        return true;
    } else {
        ESP_LOGE(OTA_TAG, "Invalid content length");
        strcpy(ota_status_message, "Invalid content length");
        esp_http_client_cleanup(client);
        return false;
    }
}

/**
 * @brief FreeRTOS task function for handling OTA updates.
 *
 * Task wrapper that safely manages memory and calls the OTA update
 * function. Handles URL memory management and cleanup on completion.
 *
 * @param[in] pvParameter  Pointer to dynamically allocated URL string.
 *
 * @note The URL parameter is freed within this task. Task deletes itself
 *       upon completion or failure.
 */
static void ota_manager_task_func(void* pvParameter) {
    ESP_LOGI(OTA_TASK_TAG, "OTA task started");
    
    // Make a local copy of the URL string to avoid memory issues
    const char* original_url = (const char*)pvParameter;
    if (original_url == NULL) {
        ESP_LOGE(OTA_TASK_TAG, "NULL URL provided to OTA task");
        ota_in_progress = false;
        vTaskDelete(NULL);
        return;
    }
    
    ESP_LOGI(OTA_TASK_TAG, "Received URL: %s", original_url);
    
    char local_url[256] = {0};
    ESP_LOGI(OTA_TASK_TAG, "Copying URL to local buffer");
    
    // Copy the URL safely
    strncpy(local_url, original_url, sizeof(local_url) - 1);
    
    // Free the original memory that was allocated with strdup
    ESP_LOGI(OTA_TASK_TAG, "Freeing original URL memory");
    free((void*)original_url);
    
    // Use the local copy
    if (strlen(local_url) > 0) {
        ESP_LOGI(OTA_TASK_TAG, "Starting OTA update with URL: %s", local_url);
        if (!ota_manager_perform_ota_update(local_url)) {
            // Only clear the flag if the update failed
            ESP_LOGE(OTA_TASK_TAG, "OTA update failed");
            ota_in_progress = false;
        }
        // Don't clear ota_in_progress on success as the device will reboot
    } else {
        ESP_LOGE(OTA_TASK_TAG, "Empty URL after copy");
        ota_in_progress = false;
    }
    
    ESP_LOGI(OTA_TASK_TAG, "OTA task ending");
    vTaskDelete(NULL);
}

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
esp_err_t ota_manager_firmware_info_handler(httpd_req_t *req)
{
    ESP_LOGI(OTA_TAG, "Getting firmware info");
    
    esp_app_desc_t *app_desc = malloc(sizeof(esp_app_desc_t));
    
    if (app_desc == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to allocate memory");
        ESP_LOGE(OTA_TAG, "Failed to allocate memory for app_desc");
        return ESP_FAIL;
    }
    
    // Get partition information
    const esp_partition_t *running = esp_ota_get_running_partition();
    if (running == NULL) {
        free(app_desc);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get running partition");
        ESP_LOGE(OTA_TAG, "Failed to get running partition");
        return ESP_FAIL;
    }
    
    // Get partition description
    esp_err_t err = esp_ota_get_partition_description(running, app_desc);
    if (err != ESP_OK) {
        free(app_desc);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get partition description");
        ESP_LOGE(OTA_TAG, "Failed to get partition description: %s", esp_err_to_name(err));
        return ESP_FAIL;
    }
    
    ESP_LOGI(OTA_TAG, "Running firmware version: %s", app_desc->version);


    // Convert SHA256 to string with more careful memory handling
    char sha256_str[sizeof(app_desc->app_elf_sha256)*2 + 1];

    // Verify buffer size is adequate before conversion
    if (sizeof(sha256_str) < sizeof(app_desc->app_elf_sha256)*2 + 1) {
        ESP_LOGE(OTA_TAG, "Buffer size too little.");
    }
    memset(sha256_str, 0, sizeof(sha256_str));

    for (int i = 0; i < sizeof(app_desc->app_elf_sha256); i++) {
        char temp[3];
        snprintf(temp, sizeof(temp), "%02x", app_desc->app_elf_sha256[i]);
        strncat(sha256_str, temp, 2);
    }
    
    ESP_LOGI(OTA_TAG, "Converted to SHA256.");

    // Create JSON response with fixed string sizes
    cJSON *root = cJSON_CreateObject();
    if (root == NULL) {
        free(app_desc);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON creation failed");
        return ESP_FAIL;
    }
    
    cJSON_AddStringToObject(root, "version", app_desc->version);
    cJSON_AddStringToObject(root, "project_name", app_desc->project_name);
    cJSON_AddStringToObject(root, "app_elf_sha256", sha256_str);
    cJSON_AddStringToObject(root, "date", app_desc->date);
    cJSON_AddStringToObject(root, "time", app_desc->time);
    cJSON_AddBoolToObject(root, "ota_in_progress", ota_in_progress);
    
    char *json_str = cJSON_Print(root);
    if (json_str == NULL) {
        cJSON_Delete(root);
        free(app_desc);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON serialization failed");
        return ESP_FAIL;
    }
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_str, strlen(json_str));
    
    // Cleanup
    free(json_str);
    cJSON_Delete(root);
    free(app_desc);
    
    return ESP_OK;
}

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
esp_err_t ota_manager_ota_update_handler(httpd_req_t *req)
{
    ESP_LOGI(OTA_HANDLER_TAG, "Received OTA update request");
    
    // Check if OTA is already in progress
    if (ota_in_progress) {
        ESP_LOGW(OTA_HANDLER_TAG, "OTA already in progress, rejecting request");
        const char *resp = "{\"status\":\"OTA already in progress\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, resp, strlen(resp));
        return ESP_OK;
    }

    ESP_LOGI(OTA_HANDLER_TAG, "Processing request with content length: %d", req->content_len);
    
    char buf[MAX_HTTP_OUTPUT_BUFFER];
    int ret, remaining = req->content_len;
    char firmware_url[256] = {0};
    
    // If no data provided, return error
    if (remaining <= 0) {
        ESP_LOGE(OTA_HANDLER_TAG, "No data provided in request");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "No data provided");
        return ESP_FAIL;
    }
    
    // Read URL from request
    ESP_LOGI(OTA_HANDLER_TAG, "Reading request data");
    while (remaining > 0) {
        // Read the data for the request
        if ((ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                ESP_LOGW(OTA_HANDLER_TAG, "Socket timeout, retrying");
                continue;
            }
            ESP_LOGE(OTA_HANDLER_TAG, "Failed to read request data: %d", ret);
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read request");
            return ESP_FAIL;
        }
        
        ESP_LOGI(OTA_HANDLER_TAG, "Read %d bytes from request", ret);
        
        // Ensure we don't exceed buffer size
        if (strlen(firmware_url) + ret < sizeof(firmware_url)) {
            strncat(firmware_url, buf, ret);
            ESP_LOGI(OTA_HANDLER_TAG, "URL buffer now contains %d bytes", strlen(firmware_url));
        } else {
            ESP_LOGE(OTA_HANDLER_TAG, "URL buffer overflow");
        }
        
        remaining -= ret;
    }
    
    ESP_LOGI(OTA_HANDLER_TAG, "Received data: %s", firmware_url);
    
    // Parse URL from JSON if needed
    ESP_LOGI(OTA_HANDLER_TAG, "Parsing JSON data");
    cJSON *root = cJSON_Parse(firmware_url);
    if (root) {
        cJSON *url_item = cJSON_GetObjectItem(root, "url");
        if (url_item && url_item->valuestring) {
            ESP_LOGI(OTA_HANDLER_TAG, "Extracted URL from JSON: %s", url_item->valuestring);
            // Clear the original buffer and copy the URL
            memset(firmware_url, 0, sizeof(firmware_url));
            strncpy(firmware_url, url_item->valuestring, sizeof(firmware_url) - 1);
        } else {
            ESP_LOGW(OTA_HANDLER_TAG, "No URL field found in JSON");
        }
        cJSON_Delete(root);
    } else {
        ESP_LOGW(OTA_HANDLER_TAG, "Not a valid JSON, using raw data as URL");
    }
    
    ESP_LOGI(OTA_HANDLER_TAG, "Final URL for OTA: %s", firmware_url);
    
    // Validate the URL (simple validation)
    if (strncmp(firmware_url, "http://", 7) != 0 && strncmp(firmware_url, "https://", 8) != 0) {
        ESP_LOGE(OTA_HANDLER_TAG, "Invalid URL format");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid URL format");
        return ESP_FAIL;
    }
    
    // Set OTA in progress flag before starting the task
    ESP_LOGI(OTA_HANDLER_TAG, "Setting OTA in progress flag");
    ota_in_progress = true;
    
    // Send success response before starting OTA
    ESP_LOGI(OTA_HANDLER_TAG, "Sending success response");
    const char *resp = "{\"status\":\"OTA update started\"}";
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp, strlen(resp));
    
    // Start the OTA process in a separate task after response is sent
    ESP_LOGI(OTA_HANDLER_TAG, "Creating OTA task");
    TaskHandle_t ota_task_handle = NULL;
    
    char* url_copy = strdup(firmware_url);
    ESP_LOGI(OTA_HANDLER_TAG, "Created URL copy at address %p", url_copy);
    
    BaseType_t task_created = xTaskCreate(
        ota_manager_task_func,
        "ota_task",
        16384,  // Increased stack size
        (void*)url_copy,
        5,
        &ota_task_handle
    );
    
    if (task_created != pdPASS) {
        ESP_LOGE(OTA_HANDLER_TAG, "Failed to create OTA task");
        free(url_copy);
        ota_in_progress = false;
        return ESP_FAIL;
    }
    
    ESP_LOGI(OTA_HANDLER_TAG, "OTA task created successfully with handle %p", ota_task_handle);
    return ESP_OK;
}

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
esp_err_t ota_manager_progress_handler(httpd_req_t *req)
{
    ESP_LOGI(OTA_PROGRESS_TAG, "Received OTA progress request");
    
    // Rate limiting for progress requests
    int64_t now = esp_timer_get_time() / 1000; // Get current time in milliseconds
    if (ota_in_progress && now - last_progress_request < 500) { // 500ms minimum between requests during OTA
        ESP_LOGW(OTA_PROGRESS_TAG, "Rate limiting OTA progress requests");
        httpd_resp_set_status(req, "503 Service Unavailable");
        httpd_resp_send(req, NULL, 0);
        return ESP_OK;
    }
    last_progress_request = now;
    
    cJSON *root = cJSON_CreateObject();
    
    // Add OTA status
    cJSON_AddBoolToObject(root, "in_progress", ota_in_progress);
    
    // Get OTA statistics if available
    if (ota_in_progress) {
        // Add progress percentage (this will be updated by the OTA task)
        cJSON_AddNumberToObject(root, "progress", ota_progress_percentage);
        cJSON_AddStringToObject(root, "status", ota_status_message);
    }
    
    char *json_str = cJSON_Print(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_str, strlen(json_str));
    
    // Cleanup
    free(json_str);
    cJSON_Delete(root);
    
    return ESP_OK;
}

/**
 * @brief Initialize the OTA manager module.
 *
 * Performs basic initialization of the OTA management system.
 * Currently just logs initialization status.
 *
 * @return ESP_OK always.
 */
esp_err_t ota_manager_init(void)
{
    ESP_LOGI(OTA_TAG, "OTA Manager initialized");
    return ESP_OK;
}

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
esp_err_t ota_manager_start_update(const char* firmware_url) // useage in the future?
{
    if (!firmware_url) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (ota_in_progress) {
        ESP_LOGW(OTA_TAG, "OTA already in progress");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Set flag and start task
    ota_in_progress = true;
    
    char* url_copy = strdup(firmware_url);
    if (!url_copy) {
        ota_in_progress = false;
        return ESP_ERR_NO_MEM;
    }
    
    BaseType_t result = xTaskCreate(
        ota_manager_task_func,
        "ota_task",
        16384,
        (void*)url_copy,
        5,
        NULL
    );
    
    if (result != pdPASS) {
        free(url_copy);
        ota_in_progress = false;
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

/**
 * @brief Check if an OTA update is currently in progress.
 *
 * @return
 *      - true if OTA update is active
 *      - false if no OTA operation is running
 */
bool ota_manager_is_in_progress(void) // useage in the future?
{
    return ota_in_progress;
}

/**
 * @brief Get the current OTA progress percentage.
 *
 * @return Progress value from 0.0 to 100.0 indicating download completion.
 */
float ota_manager_get_progress(void)
{
    return ota_progress_percentage;
}

/**
 * @brief Get the current OTA status message.
 *
 * @return Pointer to string describing the current OTA operation stage.
 */
const char* ota_manager_get_status_message(void)
{
    return ota_status_message;
}