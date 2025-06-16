/**
 * @file wifi_manager.h
 * @brief WiFi manager API for connection and credentials
 * 
 * Copyright (c) 2025 Arttu Tuominen. All rights reserved.
 * 
 * This software is licensed under commercial terms.
 * See LICENSE file for complete license terms.
 * 
 * NOTICE: This file contains proprietary information. Unauthorized 
 * distribution or use is strictly prohibited.
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_event.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define MAXIMUM_RETRY 5

// Callback function types
typedef void (*wifi_connected_cb_t)(void);
typedef void (*wifi_disconnected_cb_t)(void);

/**
 * @brief Initialize the WiFi manager system.
 *
 * Sets up WiFi infrastructure including event groups, network interface,
 * WiFi stack initialization, and event handler registration.
 *
 * @return
 *      - ESP_OK on successful WiFi manager initialization
 *      - ESP_FAIL on event group creation failure
 *      - ESP_ERR_* on WiFi stack or event handler registration failure
 */
esp_err_t wifi_manager_init(void);

/**
 * @brief Check current WiFi connection status.
 *
 * @return
 *      - true if WiFi is connected and has IP address
 *      - false if WiFi is disconnected or connecting
 */
bool wifi_manager_is_connected(void);

/**
 * @brief Disconnect from current WiFi network.
 *
 * Initiates disconnection from the currently connected WiFi network
 * and updates internal connection status.
 *
 * @return
 *      - ESP_OK on successful disconnection initiation
 *      - ESP_FAIL if WiFi manager not initialized
 */
esp_err_t wifi_manager_disconnect(void);

/**
 * @brief Retrieve saved WiFi credentials from NVS storage.
 *
 * Loads previously stored WiFi SSID and password from non-volatile
 * storage for automatic connection on device startup.
 *
 * @param[out] out_ssid         Buffer to store retrieved SSID.
 * @param[in]  ssid_buf_size    Size of the SSID buffer.
 * @param[out] out_password     Buffer to store retrieved password.
 * @param[in]  password_buf_size Size of the password buffer.
 *
 * @return
 *      - ESP_OK on successful credential retrieval
 *      - ESP_FAIL if no saved credentials exist or retrieval fails
 */
esp_err_t wifi_manager_get_saved_credentials(char* out_ssid, size_t ssid_buf_size, char* out_password, size_t password_buf_size);

/**
 * @brief Legacy WiFi connection function.
 *
 * Compatibility wrapper for existing code that uses the old WiFi
 * connection interface.
 *
 * @param[in] ssid  WiFi network SSID.
 * @param[in] pass  WiFi network password.
 *
 * @note Use wifi_manager_connect() for new code.
 */
void wifi_connect(const char *ssid, const char *pass);

/* === WiFi credentials management === */
/**
 * @brief Set new WiFi credentials received from BLE.
 *
 * Stores new WiFi credentials in JSON format to the internal buffer
 * for processing by the credentials handler task.
 *
 * @param[in] json_credentials  JSON string containing SSID and password.
 *
 * @note Credentials are processed asynchronously by the handler task.
 *       JSON format expected: {"ssid":"network_name","password":"network_pass"}
 */
void wifi_manager_set_new_credentials(const char *json_credentials);

/**
 * @brief Check if new WiFi credentials are pending processing.
 *
 * @return
 *      - true if new credentials are available for processing
 *      - false if no new credentials are pending
 */
bool wifi_manager_has_new_credentials(void);

/**
 * @brief FreeRTOS task for handling new WiFi credentials.
 *
 * Background task that monitors for new WiFi credentials from BLE,
 * parses JSON format, attempts connection, and saves successful
 * credentials to NVS storage.
 *
 * @param[in] pvParameters  Unused task parameter (pass NULL).
 *
 * @note This task runs continuously and handles credential processing
 *       asynchronously. Sends device info via BLE after successful connection.
 */
void wifi_manager_handle_new_credentials_task(void *pvParameters);

/**
 * @brief Set callback functions for WiFi events.
 *
 * Registers application callback functions to be called on WiFi
 * connection and disconnection events.
 *
 * @param[in] on_connect     Callback function for connection events.
 * @param[in] on_disconnect  Callback function for disconnection events.
 */
void wifi_manager_set_callbacks(wifi_connected_cb_t on_connect, wifi_disconnected_cb_t on_disconnect);

/**
 * @brief Get the WiFi event group handle.
 *
 * Provides access to the internal event group for advanced WiFi
 * event handling by application code.
 *
 * @return Handle to the WiFi event group.
 */
EventGroupHandle_t wifi_manager_get_event_group(void);

#endif // WIFI_MANAGER_H
