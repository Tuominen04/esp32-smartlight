/**
 * @file wifi_manager.c
 * @brief WiFi manager implementation for connection and credentials
 * 
 * Copyright (c) 2025 Arttu Tuominen. All rights reserved.
 * 
 * This software is licensed under commercial terms.
 * See LICENSE file for complete license terms.
 * 
 * NOTICE: This file contains proprietary information. Unauthorized 
 * distribution or use is strictly prohibited.
 */

#include "wifi_manager.h"
#include <string.h>
#include <stdio.h>
#include "cJSON.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

// Include device_info for saving credentials
#include "device_info.h"
#include "nvs_manager.h"

static const char *WIFI_TAG = "WIFI_MANAGER";

/** Event group handle for WiFi connection state management. */
static EventGroupHandle_t s_wifi_event_group;

/** Current retry count for WiFi connection attempts. */
static int s_retry_num = 0;

/** Flag indicating whether WiFi manager has been initialized. */
static bool s_wifi_initialized = false;

/** Flag indicating current WiFi connection status. */
static bool s_wifi_connected = false;

/** Buffer for storing incoming WiFi credentials from BLE. */
static char s_wifi_credentials_buffer[256] = {0};

/** Current length of data in the WiFi credentials buffer. */
static size_t s_wifi_credentials_len = 0;

/** Flag indicating new WiFi credentials are available for processing. */
static bool s_new_wifi_credentials = false;

/** WiFi station configuration structure for connection parameters. */
static wifi_config_t s_wifi_config = {
    .sta = {
        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
    },
};

/** Callback function pointer for WiFi connection events. */
static wifi_connected_cb_t s_connected_callback = NULL;

/** Callback function pointer for WiFi disconnection events. */
static wifi_disconnected_cb_t s_disconnected_callback = NULL;

// Forward declarations
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

/**
 * @brief Connect to WiFi network with provided credentials.
 *
 * Initiates connection to the specified WiFi network using the given
 * SSID and password. Resets retry counter and disconnects from any
 * existing connection before attempting new connection.
 *
 * @param[in] ssid      WiFi network SSID to connect to.
 * @param[in] password  WiFi network password for authentication.
 *
 * @return
 *      - ESP_OK on successful connection initiation
 *      - ESP_FAIL on invalid parameters or uninitialized manager
 *      - ESP_ERR_* on WiFi configuration or connection failure
 *
 * @note This function initiates connection asynchronously; use event
 *       callbacks or wifi_manager_is_connected() to check status.
 */
static esp_err_t wifi_manager_connect(const char *ssid, const char *password)
{
    if (!s_wifi_initialized) {
        ESP_LOGE(WIFI_TAG, "WiFi manager not initialized");
        return ESP_FAIL;
    }

    if (!ssid || !password) {
        ESP_LOGE(WIFI_TAG, "Invalid SSID or password");
        return ESP_FAIL;
    }

    ESP_LOGI(WIFI_TAG, "Connecting to WiFi SSID: %s", ssid);

    // Reset retry counter
    s_retry_num = 0;
    s_wifi_connected = false;

    // Disconnect if already connected
    esp_wifi_disconnect();
    vTaskDelay(pdMS_TO_TICKS(100));

    // Copy credentials to config
    memset(&s_wifi_config.sta.ssid, 0, sizeof(s_wifi_config.sta.ssid));
    memset(&s_wifi_config.sta.password, 0, sizeof(s_wifi_config.sta.password));
    
    strncpy((char *)s_wifi_config.sta.ssid, ssid, sizeof(s_wifi_config.sta.ssid) - 1);
    strncpy((char *)s_wifi_config.sta.password, password, sizeof(s_wifi_config.sta.password) - 1);

    // Set configuration and start
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &s_wifi_config));
    
    esp_err_t err = esp_wifi_start();
    if (err != ESP_OK && err != ESP_ERR_WIFI_NOT_STOPPED) {
        ESP_LOGE(WIFI_TAG, "Failed to start WiFi: %s", esp_err_to_name(err));
        return err;
    }

    // Connect
    err = esp_wifi_connect();
    if (err != ESP_OK) {
        ESP_LOGE(WIFI_TAG, "Failed to connect to WiFi: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(WIFI_TAG, "WiFi connection initiated, waiting for result...");
    return ESP_OK;
}

/**
 * @brief Internal WiFi event handler.
 *
 * Handles WiFi and IP events from the ESP-IDF WiFi stack, managing
 * connection state, retry logic, and callback invocation.
 *
 * @param[in] arg        User argument (unused).
 * @param[in] event_base Event base (WIFI_EVENT or IP_EVENT).
 * @param[in] event_id   Specific event ID within the event base.
 * @param[in] event_data Event-specific data structure.
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(WIFI_TAG, "WiFi station started");
                break;

            case WIFI_EVENT_STA_DISCONNECTED: {
                wifi_event_sta_disconnected_t* disconnected = (wifi_event_sta_disconnected_t*) event_data;
                ESP_LOGW(WIFI_TAG, "WiFi disconnected, reason: %d", disconnected->reason);
                
                s_wifi_connected = false;
                
                if (s_retry_num < MAXIMUM_RETRY) {
                    esp_wifi_connect();
                    s_retry_num++;
                    ESP_LOGW(WIFI_TAG, "Retrying connection (%d/%d)", s_retry_num, MAXIMUM_RETRY);
                } else {
                    ESP_LOGE(WIFI_TAG, "WiFi connection failed after %d retries", MAXIMUM_RETRY);
                    xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                    
                    if (s_disconnected_callback) {
                        s_disconnected_callback();
                    }
                }
                break;
            }

            default:
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(WIFI_TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        
        s_retry_num = 0;
        s_wifi_connected = true;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        
        if (s_connected_callback) {
            s_connected_callback();
        }

        if (!s_new_wifi_credentials) {
            ESP_LOGI(WIFI_TAG, "Connected with saved credentials, sending device info via BLE");
            send_device_info_via_ble();
        }
    }
}

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
esp_err_t wifi_manager_init(void)
{
    if (s_wifi_initialized) {
        ESP_LOGW(WIFI_TAG, "WiFi manager already initialized");
        return ESP_OK;
    }

    ESP_LOGI(WIFI_TAG, "Initializing WiFi manager");

    // Create event group
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) {
        ESP_LOGE(WIFI_TAG, "Failed to create event group");
        return ESP_FAIL;
    }

    // Initialize network interface (only if not already done)
    esp_err_t ret = esp_netif_init();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(WIFI_TAG, "Failed to initialize netif: %s", esp_err_to_name(ret));
        return ret;
    }

    // Create default event loop (only if not already created)
    ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(WIFI_TAG, "Failed to create event loop: %s", esp_err_to_name(ret));
        return ret;
    }

    esp_netif_t *sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (sta_netif == NULL) {
        esp_netif_create_default_wifi_sta();
    }

    // Initialize WiFi
    // Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(WIFI_TAG, "Failed to initialize WiFi: %s", esp_err_to_name(ret));
        return ret;
    }

    // Register event handlers
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    
    ret = esp_event_handler_instance_register(WIFI_EVENT,
                                           ESP_EVENT_ANY_ID,
                                           &wifi_event_handler,
                                           NULL,
                                           &instance_any_id);
    if (ret != ESP_OK) {
        ESP_LOGE(WIFI_TAG, "Failed to register WiFi event handler: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = esp_event_handler_instance_register(IP_EVENT,
                                           IP_EVENT_STA_GOT_IP,
                                           &wifi_event_handler,
                                           NULL,
                                           &instance_got_ip);
    if (ret != ESP_OK) {
        ESP_LOGE(WIFI_TAG, "Failed to register IP event handler: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK) {
        ESP_LOGE(WIFI_TAG, "Failed to set WiFi mode: %s", esp_err_to_name(ret));
        return ret;
    }
    
    s_wifi_initialized = true;
    ESP_LOGI(WIFI_TAG, "WiFi manager initialized successfully");
    
    return ESP_OK;
}

/**
 * @brief Check current WiFi connection status.
 *
 * @return
 *      - true if WiFi is connected and has IP address
 *      - false if WiFi is disconnected or connecting
 */
bool wifi_manager_is_connected(void)
{
    return s_wifi_connected;
}

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
esp_err_t wifi_manager_disconnect(void)
{
    if (!s_wifi_initialized) {
        return ESP_FAIL;
    }

    if (!s_wifi_connected) {
        ESP_LOGW(WIFI_TAG, "WiFi is not connected, nothing to disconnect");
        return ESP_OK; // Already disconnected
    }

    ESP_LOGI(WIFI_TAG, "Disconnecting from WiFi");
    s_wifi_connected = false;
    return esp_wifi_disconnect();
}

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
esp_err_t wifi_manager_get_saved_credentials(char* out_ssid, size_t ssid_buf_size, char* out_password, size_t password_buf_size) {
    esp_err_t err = nvs_manager_get_wifi_credentials(out_ssid, ssid_buf_size, out_password, password_buf_size);

    if (err == ESP_OK) {
        ESP_LOGI(WIFI_TAG, "Found WiFi saved credentials.");
        return ESP_OK;
    } else {
        ESP_LOGI(WIFI_TAG, "No saved credentials: %s", esp_err_to_name(err));
        return ESP_FAIL;
    }
}

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
void wifi_manager_set_new_credentials(const char *json_credentials)
{
    if (!json_credentials || json_credentials[0] == '\0') {
        ESP_LOGE(WIFI_TAG, "NULL credentials provided");
        s_new_wifi_credentials = false;
        return;
    }

    size_t len = strlen(json_credentials);
    if (len >= sizeof(s_wifi_credentials_buffer)) {
        ESP_LOGE(WIFI_TAG, "Credentials too long: %d bytes", len);
        s_new_wifi_credentials = false;
        return;
    }

    // Copy credentials to buffer
    memset(s_wifi_credentials_buffer, 0, sizeof(s_wifi_credentials_buffer));
    strncpy(s_wifi_credentials_buffer, json_credentials, sizeof(s_wifi_credentials_buffer) - 1);
    s_wifi_credentials_len = len;
    s_new_wifi_credentials = true;

    ESP_LOGI(WIFI_TAG, "New WiFi credentials received");
}

/**
 * @brief Check if new WiFi credentials are pending processing.
 *
 * @return
 *      - true if new credentials are available for processing
 *      - false if no new credentials are pending
 */
bool wifi_manager_has_new_credentials(void)
{
    return s_new_wifi_credentials;
}

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
void wifi_manager_handle_new_credentials_task(void *pvParameters)
{
    ESP_LOGI(WIFI_TAG, "WiFi credentials handler task started");

    while (1) {
        if (s_new_wifi_credentials && s_wifi_credentials_len > 0) {
            ESP_LOGI(WIFI_TAG, "Processing new WiFi credentials");

            // Parse JSON
            cJSON *root = cJSON_Parse(s_wifi_credentials_buffer);
            if (root) {
                cJSON *ssid_item = cJSON_GetObjectItem(root, "ssid");
                cJSON *pass_item = cJSON_GetObjectItem(root, "password");

                if (ssid_item && pass_item && 
                    ssid_item->valuestring && pass_item->valuestring) {
                    
                    const char *ssid = ssid_item->valuestring;
                    const char *password = pass_item->valuestring;

                    ESP_LOGI(WIFI_TAG, "Connecting to new SSID: %s", ssid);

                    // Attempt connection
                    esp_err_t result = wifi_manager_connect(ssid, password);
                    if (result == ESP_OK) {
                        // Wait for connection result
                        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                pdFALSE, pdFALSE,
                                pdMS_TO_TICKS(15000)); // 15 second timeout

                        if (bits & WIFI_CONNECTED_BIT) {
                            ESP_LOGI(WIFI_TAG, "Successfully connected to new WiFi");
                            
                            device_manager_save_device_info(ssid, password);
                            
                            ESP_LOGI(WIFI_TAG, "Sending device info via BLE after saving credentials");
                            send_device_info_via_ble();
                            
                        } else {
                            ESP_LOGE(WIFI_TAG, "Failed to connect to new WiFi");
                        }
                    }
                } else {
                    ESP_LOGW(WIFI_TAG, "Invalid JSON credentials format");
                }

                cJSON_Delete(root);
            } else {
                ESP_LOGW(WIFI_TAG, "Failed to parse credentials JSON");
            }

            // Reset credentials flag
            s_new_wifi_credentials = false;
            memset(s_wifi_credentials_buffer, 0, sizeof(s_wifi_credentials_buffer));
            s_wifi_credentials_len = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief Set callback functions for WiFi events.
 *
 * Registers application callback functions to be called on WiFi
 * connection and disconnection events.
 *
 * @param[in] on_connect     Callback function for connection events.
 * @param[in] on_disconnect  Callback function for disconnection events.
 */
void wifi_manager_set_callbacks(wifi_connected_cb_t on_connect, wifi_disconnected_cb_t on_disconnect)
{
    s_connected_callback = on_connect;
    s_disconnected_callback = on_disconnect;
}

/**
 * @brief Get the WiFi event group handle.
 *
 * Provides access to the internal event group for advanced WiFi
 * event handling by application code.
 *
 * @return Handle to the WiFi event group.
 */
EventGroupHandle_t wifi_manager_get_event_group(void)
{
    return s_wifi_event_group;
}

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
void wifi_connect(const char *ssid, const char *pass)
{
    wifi_manager_connect(ssid, pass);
}
