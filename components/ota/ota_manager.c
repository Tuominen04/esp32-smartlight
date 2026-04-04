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
#include "esp_crt_bundle.h"
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

#define OTA_STATUS_MSG_MAX_LEN 64
#define OTA_MIN_PROGRESS_INTERVAL_MS 500

/** Flag indicating whether an OTA update is currently in progress. */
static bool ota_in_progress = false;

/** Current progress percentage of the OTA update (0.0 to 100.0). */
static float ota_progress_percentage = 0.0;

/** Current status message describing the OTA update stage. */
static char ota_status_message[OTA_STATUS_MSG_MAX_LEN] = "Initializing...";

/** Timestamp of the last progress request to implement rate limiting. */
static int64_t last_progress_request = 0;

/**
 * @brief Update OTA status message safely.
 *
 * @param[in] message  Null-terminated status message.
 */
static void ota_set_status_message(const char *message)
{
  if (message == NULL) {
    ota_status_message[0] = '\0';
    return;
  }

  snprintf(ota_status_message, sizeof(ota_status_message), "%s", message);
}

/**
 * @brief Check whether URL uses HTTPS scheme.
 *
 * @param[in] url  Candidate URL.
 *
 * @return true if URL starts with "https://", otherwise false.
 */
static bool ota_url_is_https(const char *url)
{
  return (url != NULL) && (strncmp(url, "https://", 8) == 0);
}

/**
 * @brief Check whether URL uses HTTP or HTTPS scheme.
 *
 * @param[in] url  Candidate URL.
 *
 * @return true if URL starts with "http://" or "https://", otherwise false.
 */
static bool ota_url_is_valid(const char *url)
{
  return (url != NULL) &&
         ((strncmp(url, "http://", 7) == 0) || (strncmp(url, "https://", 8) == 0));
}

/**
 * @brief Send JSON HTTP response with error checking.
 *
 * @param[in] req   HTTP request.
 * @param[in] body  JSON response body.
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_INVALID_ARG on invalid arguments
 *      - ESP_ERR_* returned by HTTP response APIs
 */
static esp_err_t ota_send_json_response(httpd_req_t *req, const char *body)
{
  if (req == NULL || body == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  esp_err_t err = httpd_resp_set_type(req, "application/json");
  if (err != ESP_OK) {
    ESP_LOGE(OTA_HANDLER_TAG, "Failed to set response type: %s", esp_err_to_name(err));
    return err;
  }

  err = httpd_resp_send(req, body, HTTPD_RESP_USE_STRLEN);
  if (err != ESP_OK) {
    ESP_LOGE(OTA_HANDLER_TAG, "Failed to send response: %s", esp_err_to_name(err));
  }
  return err;
}

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
  case HTTP_EVENT_ON_HEADERS_COMPLETE:
    ESP_LOGI(OTA_HTTP_TAG, "All headers received");
    break;
  case HTTP_EVENT_ON_STATUS_CODE:
    ESP_LOGI(OTA_HTTP_TAG, "Received status code event");
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
  if (!ota_url_is_valid(firmware_url)) {
    ESP_LOGE(OTA_TAG, "Rejected OTA URL: must start with http:// or https://");
    ota_set_status_message("Invalid firmware URL");
    return false;
  }

  if (!ota_url_is_https(firmware_url)) {
    ESP_LOGW(OTA_TAG, "OTA URL is HTTP (not HTTPS) - only use on a trusted local network");
  }

  ESP_LOGI(OTA_TAG, "Starting OTA update process");

  // Reset progress indicators
  ota_progress_percentage = 0.0;
  ota_set_status_message("Connecting to server...");

  esp_http_client_config_t config = {
    .url = firmware_url,
    .event_handler = ota_manager_http_event_handler,
    .buffer_size = OTA_BUF_SIZE,
    .timeout_ms = HTTP_RESPONSE_TIMEOUT_MS,
    .skip_cert_common_name_check = false,
    .cert_pem = NULL,
    .crt_bundle_attach = ota_url_is_https(firmware_url) ? esp_crt_bundle_attach : NULL,
  };
  
  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (client == NULL) {
    ESP_LOGE(OTA_TAG, "Failed to initialize HTTP client");
    ota_set_status_message("Failed to initialize HTTP client");
    return false;
  }

  // Open connection
  ota_set_status_message("Opening HTTP connection...");
  esp_err_t err = esp_http_client_open(client, 0);
  if (err != ESP_OK) {
    ESP_LOGE(OTA_TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    ota_set_status_message("Failed to open HTTP connection");
    esp_http_client_cleanup(client);
    return false;
  }

  // Get content length
  ota_set_status_message("Fetching firmware size...");
  int content_length = esp_http_client_fetch_headers(client);
  if (content_length < 0) {
    ESP_LOGE(OTA_TAG, "Failed to get firmware size");
    ota_set_status_message("Failed to get firmware size");
    esp_http_client_cleanup(client);
    return false;
  }

  if (content_length <= 0) {
    ESP_LOGE(OTA_TAG, "Invalid firmware size: %d", content_length);
    ota_set_status_message("Invalid firmware size");
    esp_http_client_cleanup(client);
    return false;
  }

  ESP_LOGI(OTA_TAG, "Firmware size: %d bytes", content_length);
  snprintf(ota_status_message, sizeof(ota_status_message), "Firmware size: %d bytes", content_length);

  // Get OTA handle
  esp_ota_handle_t ota_handle = 0;
  const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);

  if (update_partition == NULL) {
    ESP_LOGE(OTA_TAG, "Failed to get update partition");
    ota_set_status_message("Failed to get update partition");
    esp_http_client_cleanup(client);
    return false;
  }

  ESP_LOGI(
      OTA_TAG,
      "Writing to partition label: %s, subtype: %d, offset: 0x%lx",
      update_partition->label,
      update_partition->subtype,
      (unsigned long)update_partition->address);
  snprintf(
      ota_status_message,
      sizeof(ota_status_message),
      "Writing to partition: %s",
      update_partition->label);

    // Begin OTA
  err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &ota_handle);
  if (err != ESP_OK) {
    ESP_LOGE(OTA_TAG, "Failed to begin OTA update: %s", esp_err_to_name(err));
    ota_set_status_message("Failed to begin OTA update");
    esp_http_client_cleanup(client);
    return false;
  }

    // Create buffer for data
  char *upgrade_data_buf = (char *)malloc(OTA_BUF_SIZE);
  if (!upgrade_data_buf) {
    ESP_LOGE(OTA_TAG, "Failed to allocate memory for upgrade data buffer");
    ota_set_status_message("Failed to allocate memory");
    esp_ota_end(ota_handle);
    esp_http_client_cleanup(client);
    return false;
  }

    // Read data and write to flash
  int binary_file_length = 0;
  int data_read;
  bool image_header_checked = false;

  while (1) {
    data_read = esp_http_client_read(client, upgrade_data_buf, OTA_BUF_SIZE);
    if (data_read < 0) {
      ESP_LOGE(OTA_TAG, "Error reading data");
      ota_set_status_message("Error reading data");
      free(upgrade_data_buf);
      esp_ota_end(ota_handle);
      esp_http_client_cleanup(client);
      return false;
    } else if (data_read == 0) {
      break;
    }

      // Check image header first
    if (!image_header_checked) {
      esp_app_desc_t new_app_info;
      if (data_read >=
          (int)(sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t))) {
          // Check current running version with the one we're trying to download
        memcpy(
            &new_app_info,
            &upgrade_data_buf[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)],
            sizeof(esp_app_desc_t));
        ESP_LOGI(OTA_TAG, "New firmware version: %s", new_app_info.version);
          
          // Get info about running firmware
        const esp_partition_t *running = esp_ota_get_running_partition();
        esp_app_desc_t running_app_info;
        if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
          ESP_LOGI(OTA_TAG, "Running firmware version: %s", running_app_info.version);
        }
        ota_set_status_message("Downloading...");
        image_header_checked = true;
      } else {
        ESP_LOGE(OTA_TAG, "Invalid image header");
        ota_set_status_message("Invalid image header");
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
      ota_set_status_message("Error writing OTA data");
      free(upgrade_data_buf);
      esp_ota_end(ota_handle);
      esp_http_client_cleanup(client);
      return false;
    }

    binary_file_length += data_read;
    ota_progress_percentage = ((float)binary_file_length / (float)content_length) * 100.0f;
    if (ota_progress_percentage > 100.0f) {
      ota_progress_percentage = 100.0f;
    }

    ESP_LOGI(OTA_TAG, "Downloaded %d/%d bytes (%.1f%%)", binary_file_length, content_length, ota_progress_percentage);
    ota_set_status_message("Downloading...");
  }

    // Clean up
  free(upgrade_data_buf);

  if (binary_file_length != content_length) {
    ESP_LOGE(OTA_TAG, "Download incomplete: %d/%d bytes", binary_file_length, content_length);
    ota_set_status_message("Download incomplete");
    esp_ota_end(ota_handle);
    esp_http_client_cleanup(client);
    return false;
  }

    // End OTA and validate
  ota_set_status_message("Validating firmware...");
  err = esp_ota_end(ota_handle);
  if (err != ESP_OK) {
    if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
      ESP_LOGE(OTA_TAG, "Firmware validation failed");
      ota_set_status_message("Firmware validation failed");
    } else {
      ESP_LOGE(OTA_TAG, "Error finalizing OTA update: %s", esp_err_to_name(err));
      ota_set_status_message("Error finalizing OTA update");
    }
    esp_http_client_cleanup(client);
    return false;
  }

  ota_set_status_message("Setting boot partition...");
  err = esp_ota_set_boot_partition(update_partition);
  if (err != ESP_OK) {
    ESP_LOGE(OTA_TAG, "Failed to set boot partition: %s", esp_err_to_name(err));
    ota_set_status_message("Failed to set boot partition");
    esp_http_client_cleanup(client);
    return false;
  }

  ESP_LOGI(OTA_TAG, "Update successful! Rebooting in 5 seconds...");
  ota_set_status_message("Update successful! Rebooting...");
  esp_http_client_cleanup(client);
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  esp_restart();
  return true;
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
static void ota_manager_task_func(void* pvParameter)
{
  ESP_LOGI(OTA_TASK_TAG, "OTA task started");
  
  // Make a local copy of the URL string to avoid memory issues
  const char* original_url = (const char*)pvParameter;
  if (original_url == NULL) {
    ESP_LOGE(OTA_TASK_TAG, "NULL URL provided to OTA task");
    ota_in_progress = false;
    vTaskDelete(NULL);
    return;
  }
  
  char local_url[256] = {0};
  ESP_LOGI(OTA_TASK_TAG, "Copying URL to local buffer");
  
  // Copy the URL safely
  snprintf(local_url, sizeof(local_url), "%s", original_url);
  
  // Free the original memory that was allocated with strdup
  ESP_LOGI(OTA_TASK_TAG, "Freeing original URL memory");
  free((void*)original_url);
  
  // Use the local copy
  if (strlen(local_url) > 0) {
    ESP_LOGI(OTA_TASK_TAG, "Starting OTA update task");
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
  if (req == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  ESP_LOGI(OTA_TAG, "Getting firmware info");

  esp_app_desc_t app_desc;
  
  // Get partition information
  const esp_partition_t *running = esp_ota_get_running_partition();
  if (running == NULL) {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get running partition");
    ESP_LOGE(OTA_TAG, "Failed to get running partition");
    return ESP_FAIL;
  }
  
  // Get partition description
  esp_err_t err = esp_ota_get_partition_description(running, &app_desc);
  if (err != ESP_OK) {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get partition description");
    ESP_LOGE(OTA_TAG, "Failed to get partition description: %s", esp_err_to_name(err));
    return ESP_FAIL;
  }

  ESP_LOGI(OTA_TAG, "Running firmware version: %s", app_desc.version);

  // Convert SHA256 to string with more careful memory handling
  char sha256_str[(sizeof(app_desc.app_elf_sha256) * 2U) + 1U] = {0};
  for (size_t i = 0; i < sizeof(app_desc.app_elf_sha256); i++) {
    snprintf(&sha256_str[i * 2U], 3, "%02x", app_desc.app_elf_sha256[i]);
  }
  
  ESP_LOGI(OTA_TAG, "Converted to SHA256.");

  // Create JSON response with fixed string sizes
  cJSON *root = cJSON_CreateObject();
  if (root == NULL) {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON creation failed");
    return ESP_FAIL;
  }

  cJSON_AddStringToObject(root, "version", app_desc.version);
  cJSON_AddStringToObject(root, "project_name", app_desc.project_name);
  cJSON_AddStringToObject(root, "app_elf_sha256", sha256_str);
  cJSON_AddStringToObject(root, "date", app_desc.date);
  cJSON_AddStringToObject(root, "time", app_desc.time);
  cJSON_AddBoolToObject(root, "ota_in_progress", ota_in_progress);

  char *json_str = cJSON_PrintUnformatted(root);
  if (json_str == NULL) {
    cJSON_Delete(root);
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "JSON serialization failed");
    return ESP_FAIL;
  }

  err = ota_send_json_response(req, json_str);

  // Cleanup
  free(json_str);
  cJSON_Delete(root);

  return err;
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
  if (req == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  ESP_LOGI(OTA_HANDLER_TAG, "Received OTA update request");
  
  // Check if OTA is already in progress
  if (ota_in_progress) {
    ESP_LOGW(OTA_HANDLER_TAG, "OTA already in progress, rejecting request");
    const char *resp = "{\"status\":\"OTA already in progress\"}";
    return ota_send_json_response(req, resp);
  }

  ESP_LOGI(OTA_HANDLER_TAG, "Processing request with content length: %d", req->content_len);

  if (req->content_len <= 0 || req->content_len >= MAX_HTTP_OUTPUT_BUFFER) {
    ESP_LOGE(OTA_HANDLER_TAG, "Invalid OTA request body length: %d", req->content_len);
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid request body length");
    return ESP_FAIL;
  }

  char request_body[MAX_HTTP_OUTPUT_BUFFER] = {0};
  int received = 0;
  char firmware_url[256] = {0};

  // Read URL from request
  ESP_LOGI(OTA_HANDLER_TAG, "Reading request data");
  while (received < req->content_len) {
    int ret = httpd_req_recv(req, request_body + received, req->content_len - received);
    if (ret <= 0) {
      if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
        ESP_LOGW(OTA_HANDLER_TAG, "Socket timeout, retrying");
        continue;
      }
      ESP_LOGE(OTA_HANDLER_TAG, "Failed to read request data: %d", ret);
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read request");
      return ESP_FAIL;
    }

    received += ret;
  }
  request_body[received] = '\0';
  
  // Parse URL from JSON if needed
  ESP_LOGI(OTA_HANDLER_TAG, "Parsing JSON data");
  cJSON *root = cJSON_Parse(request_body);
  if (root) {
    cJSON *url_item = cJSON_GetObjectItem(root, "url");
    if (url_item && url_item->valuestring) {
      size_t url_len = strnlen(url_item->valuestring, sizeof(firmware_url));
      if (url_len >= sizeof(firmware_url)) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Firmware URL too long");
        return ESP_FAIL;
      }
      memcpy(firmware_url, url_item->valuestring, url_len);
      firmware_url[url_len] = '\0';
    } else {
      ESP_LOGW(OTA_HANDLER_TAG, "No URL field found in JSON");
    }
    cJSON_Delete(root);
  } else {
    size_t url_len = strnlen(request_body, sizeof(firmware_url));
    if (url_len >= sizeof(firmware_url)) {
      httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Firmware URL too long");
      return ESP_FAIL;
    }
    memcpy(firmware_url, request_body, url_len);
    firmware_url[url_len] = '\0';
  }

  if (firmware_url[0] == '\0') {
    ESP_LOGE(OTA_HANDLER_TAG, "Firmware URL is empty");
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Firmware URL is required");
    return ESP_FAIL;
  }

  if (!ota_url_is_valid(firmware_url)) {
    ESP_LOGE(OTA_HANDLER_TAG, "Rejected invalid firmware URL: must start with http:// or https://");
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Firmware URL must use http:// or https://");
    return ESP_FAIL;
  }

  if (!ota_url_is_https(firmware_url)) {
    ESP_LOGW(OTA_HANDLER_TAG, "OTA URL is HTTP (not HTTPS) - only use on a trusted local network");
  }

  // Set OTA in progress flag before starting the task
  ESP_LOGI(OTA_HANDLER_TAG, "Setting OTA in progress flag");
  ota_in_progress = true;

  // Start the OTA process in a separate task after response is sent
  ESP_LOGI(OTA_HANDLER_TAG, "Creating OTA task");
  TaskHandle_t ota_task_handle = NULL;

  char* url_copy = strdup(firmware_url);
  if (url_copy == NULL) {
    ota_in_progress = false;
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to allocate URL buffer");
    return ESP_ERR_NO_MEM;
  }
  
  BaseType_t task_created = xTaskCreate(
    ota_manager_task_func,
    "ota_task",
    OTA_TASK_STACK_SIZE,
    (void*)url_copy,
    OTA_TASK_PRIORITY,
    &ota_task_handle
  );

  if (task_created != pdPASS) {
    ESP_LOGE(OTA_HANDLER_TAG, "Failed to create OTA task");
    free(url_copy);
    ota_in_progress = false;
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create OTA task");
    return ESP_FAIL;
  }

  ota_progress_percentage = 0.0f;
  ota_set_status_message("OTA update started");

  ESP_LOGI(OTA_HANDLER_TAG, "Sending success response");
  const char *resp = "{\"status\":\"OTA update started\"}";
  esp_err_t send_err = ota_send_json_response(req, resp);
  if (send_err != ESP_OK) {
    ESP_LOGW(OTA_HANDLER_TAG, "OTA task started but response could not be sent");
    return send_err;
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
  if (req == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  // Rate limiting for progress requests
  int64_t now = esp_timer_get_time() / 1000; // Get current time in milliseconds
  if (ota_in_progress && now - last_progress_request < OTA_MIN_PROGRESS_INTERVAL_MS) {
    ESP_LOGW(OTA_PROGRESS_TAG, "Rate limiting OTA progress requests");
    esp_err_t err = httpd_resp_set_status(req, "503 Service Unavailable");
    if (err != ESP_OK) {
      return err;
    }
    return httpd_resp_send(req, NULL, 0);
  }
  last_progress_request = now;

  cJSON *root = cJSON_CreateObject();
  if (root == NULL) {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to build progress response");
    return ESP_FAIL;
  }

  // Add OTA status
  cJSON_AddBoolToObject(root, "in_progress", ota_in_progress);
  cJSON_AddNumberToObject(root, "progress", ota_progress_percentage);
  cJSON_AddStringToObject(root, "status", ota_status_message);

  char *json_str = cJSON_PrintUnformatted(root);
  if (json_str == NULL) {
    cJSON_Delete(root);
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to serialize progress response");
    return ESP_FAIL;
  }

  esp_err_t err = ota_send_json_response(req, json_str);

  // Cleanup
  free(json_str);
  cJSON_Delete(root);

  return err;
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
esp_err_t ota_manager_start_update(const char* firmware_url)
{
  if (!firmware_url) {
    return ESP_ERR_INVALID_ARG;
  }

  if (!ota_url_is_valid(firmware_url)) {
    ESP_LOGE(OTA_TAG, "Rejected invalid firmware URL: must start with http:// or https://");
    return ESP_ERR_INVALID_ARG;
  }

  if (!ota_url_is_https(firmware_url)) {
    ESP_LOGW(OTA_TAG, "OTA URL is HTTP (not HTTPS) - only use on a trusted local network");
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
    OTA_TASK_STACK_SIZE,
    (void*)url_copy,
    OTA_TASK_PRIORITY,
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
bool ota_manager_is_in_progress(void)
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