/**
 * @file http_server.c
 * @brief HTTP server implementation for light control and OTA
 * 
 * Copyright (c) 2025 Arttu Tuominen. All rights reserved.
 * 
 * This software is licensed under commercial terms.
 * See LICENSE file for complete license terms.
 * 
 * NOTICE: This file contains proprietary information. Unauthorized 
 * distribution or use is strictly prohibited.
 */

#include "http_server.h"
#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include "esp_http_server.h"

// Include other modules for handlers
#include "../gpio/gpio_control.h"
#include "../ota/ota_manager.h"
#include "../device/device_info.h"

static const char *HTTP_TAG = "HTTP_SERVER";

/** HTTP server handle for managing the web server instance. */
static httpd_handle_t s_server = NULL;

/** Flag indicating whether the HTTP server is currently running. */
static bool s_server_running = false;

/** TCP port number on which the HTTP server listens for connections. */
static uint16_t s_server_port = 80;

/** Stack size allocated for HTTP server tasks in bytes. */
static size_t s_stack_size = 8192;

/** Maximum number of simultaneous HTTP connections allowed. */
static int s_max_connections = 3;

/**
 * @brief HTTP handler for light status GET requests.
 *
 * Handles GET requests to the `/light` endpoint, returning the current
 * device number and light state in JSON format. This provides a RESTful
 * interface for querying the current status of the light device.
 *
 * @param[in] req  HTTP request structure containing client request data.
 *
 * @return
 *      - ESP_OK on successful response transmission
 *      - ESP_FAIL on error during response handling
 *
 * @note Response format: {"device":1,"state":"on"|"off"}
 */
static esp_err_t status_get_handler(httpd_req_t *req)
{
    char json_resp[100];
    sprintf(json_resp, "{\"device\":%d,\"state\":\"%s\"}", 
            DEVICE_NUMBER, gpio_get_light_state() ? "on" : "off");
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_resp, strlen(json_resp));
    
    ESP_LOGI(HTTP_TAG, "Status request served");
    return ESP_OK;
}

/**
 * @brief HTTP handler for light toggle PUT requests.
 *
 * Handles PUT requests to the `/toggle` endpoint, toggling the current
 * light state and returning the new state as plain text. This provides
 * a simple interface for switching the light on/off remotely.
 *
 * @param[in] req  HTTP request structure containing client request data.
 *
 * @return
 *      - ESP_OK on successful light toggle and response
 *      - ESP_FAIL on error during processing
 *
 * @note Response is plain text: "ON" or "OFF" based on new light state.
 */
static esp_err_t light_toggle_handler(httpd_req_t *req)
{
    // Toggle the light state
    gpio_toggle_light();

    // Get the current state
    bool current_state = gpio_get_light_state();
    
    ESP_LOGI(HTTP_TAG, "Light toggled to %s", current_state ? "ON" : "OFF");
    
    // Return a simple response
    const char *resp = current_state ? "ON" : "OFF";
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, resp, strlen(resp));
    
    return ESP_OK;
}

/**
 * @brief Register HTTP routes for light control operations.
 *
 * Registers the following light control endpoints with the HTTP server:
 * - GET /light: Returns current device and light state in JSON format
 * - PUT /toggle: Toggles the light state and returns new state as text
 *
 * @return
 *      - ESP_OK on successful route registration
 *      - ESP_ERR_INVALID_STATE if server is not started
 *      - ESP_ERR_* on route registration failure
 *
 * @note This function requires the HTTP server to be started before calling.
 */
static esp_err_t http_server_register_light_routes(void)
{
    if (s_server == NULL) {
        ESP_LOGE(HTTP_TAG, "Server not started, cannot register routes");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Status endpoint
    httpd_uri_t status_uri = {
        .uri       = "/light",
        .method    = HTTP_GET,
        .handler   = status_get_handler,
        .user_ctx  = NULL
    };
    esp_err_t result = httpd_register_uri_handler(s_server, &status_uri);
    if (result != ESP_OK) {
        ESP_LOGE(HTTP_TAG, "Failed to register status handler");
        return result;
    }

    // Toggle endpoint
    httpd_uri_t toggle_uri = {
        .uri       = "/toggle",
        .method    = HTTP_PUT,
        .handler   = light_toggle_handler,
        .user_ctx  = NULL
    };
    result = httpd_register_uri_handler(s_server, &toggle_uri);
    if (result != ESP_OK) {
        ESP_LOGE(HTTP_TAG, "Failed to register toggle handler");
        return result;
    }
    
    ESP_LOGI(HTTP_TAG, "Light control routes registered");
    return ESP_OK;
}

/**
 * @brief Register HTTP routes for OTA (Over-The-Air) update operations.
 *
 * Registers the following OTA management endpoints with the HTTP server:
 * - GET /ota/firmware-info: Returns current firmware information
 * - POST /ota/update: Initiates OTA firmware update process
 * - GET /ota/progress: Returns OTA update progress and status
 *
 * @return
 *      - ESP_OK on successful route registration
 *      - ESP_ERR_INVALID_STATE if server is not started
 *      - ESP_ERR_* on route registration failure
 *
 * @note The actual OTA handlers are implemented in the ota_manager module.
 */
static esp_err_t http_server_register_ota_routes(void)
{
    if (s_server == NULL) {
        ESP_LOGE(HTTP_TAG, "Server not started, cannot register routes");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Firmware info endpoint
    httpd_uri_t firmware_info_uri = {
        .uri       = "/ota/firmware-info",
        .method    = HTTP_GET,
        .handler   = ota_manager_firmware_info_handler,
        .user_ctx  = NULL
    };
    esp_err_t result = httpd_register_uri_handler(s_server, &firmware_info_uri);
    if (result != ESP_OK) {
        ESP_LOGE(HTTP_TAG, "Failed to register firmware info handler");
        return result;
    }
    
    // OTA update endpoint
    httpd_uri_t ota_update_uri = {
        .uri       = "/ota/update",
        .method    = HTTP_POST,
        .handler   = ota_manager_ota_update_handler,
        .user_ctx  = NULL
    };
    result = httpd_register_uri_handler(s_server, &ota_update_uri);
    if (result != ESP_OK) {
        ESP_LOGE(HTTP_TAG, "Failed to register OTA update handler");
        return result;
    }
    
    // OTA progress endpoint
    httpd_uri_t ota_progress_uri = {
        .uri       = "/ota/progress",
        .method    = HTTP_GET,
        .handler   = ota_manager_progress_handler,
        .user_ctx  = NULL
    };
    result = httpd_register_uri_handler(s_server, &ota_progress_uri);
    if (result != ESP_OK) {
        ESP_LOGE(HTTP_TAG, "Failed to register OTA progress handler");
        return result;
    }
    
    ESP_LOGI(HTTP_TAG, "OTA routes registered");
    return ESP_OK;
}

/**
 * @brief Register all available HTTP routes with the server.
 *
 * Convenience function that registers both light control and OTA management
 * routes in a single call. This ensures all application endpoints are
 * available for client requests.
 *
 * @return
 *      - ESP_OK on successful registration of all routes
 *      - ESP_ERR_* on failure during route registration
 *
 * @note If any route registration fails, the function returns immediately
 *       with the error code, potentially leaving some routes unregistered.
 */
static esp_err_t http_server_register_all_routes(void)
{
    esp_err_t result;
    
    result = http_server_register_light_routes();
    if (result != ESP_OK) {
        return result;
    }
    
    result = http_server_register_ota_routes();
    if (result != ESP_OK) {
        return result;
    }
    
    ESP_LOGI(HTTP_TAG, "All routes registered successfully");
    return ESP_OK;
}

/**
 * @brief Initialize the HTTP server module.
 *
 * Performs basic initialization of the HTTP server module without starting
 * the actual server. This allows configuration to be set before the server
 * begins accepting connections.
 *
 * @return
 *      - ESP_OK on successful initialization
 *      - ESP_FAIL if server is already initialized
 *
 * @note This function only initializes the module state; use http_server_start()
 *       to begin accepting HTTP connections.
 */
esp_err_t http_server_init(void)
{
    if (s_server != NULL) {
        ESP_LOGW(HTTP_TAG, "HTTP server already initialized");
        return ESP_OK;
    }
    
    ESP_LOGI(HTTP_TAG, "HTTP server initialized");
    return ESP_OK;
}

/**
 * @brief Start the HTTP server and begin accepting connections.
 *
 * Creates and starts the HTTP server with the configured parameters,
 * then registers all defined routes. The server will begin accepting
 * client connections on the specified port after successful startup.
 *
 * @return
 *      - ESP_OK on successful server start and route registration
 *      - ESP_FAIL if server is already running
 *      - ESP_ERR_* on HTTP server creation or route registration failure
 *
 * @note The server uses the configuration set via http_server_set_* functions.
 *       All routes are automatically registered upon successful startup.
 */
esp_err_t http_server_start(void)
{
    if (s_server != NULL) {
        ESP_LOGW(HTTP_TAG, "HTTP server already running");
        return ESP_OK;
    }
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    
    // Apply custom configuration
    config.server_port = s_server_port;
    config.stack_size = s_stack_size;
    config.max_open_sockets = s_max_connections;
    config.lru_purge_enable = true;  // Enable LRU socket purging
    
    ESP_LOGI(HTTP_TAG, "Starting HTTP server on port %d", config.server_port);
    
    esp_err_t result = httpd_start(&s_server, &config);
    if (result == ESP_OK) {
        s_server_running = true;
        
        // Register all routes
        esp_err_t route_result = http_server_register_all_routes();
        if (route_result != ESP_OK) {
            ESP_LOGE(HTTP_TAG, "Failed to register routes");
            http_server_stop();
            return route_result;
        }
        
        ESP_LOGI(HTTP_TAG, "HTTP server started successfully");
        return ESP_OK;
    } else {
        ESP_LOGE(HTTP_TAG, "Failed to start HTTP server: %s", esp_err_to_name(result));
        s_server = NULL;
        return result;
    }
}

/**
 * @brief Stop the HTTP server and close all connections.
 *
 * Gracefully shuts down the HTTP server, closing all active client
 * connections and freeing associated resources. The server will stop
 * accepting new connections immediately.
 *
 * @return
 *      - ESP_OK on successful server shutdown
 *      - ESP_FAIL if server was not running
 *      - ESP_ERR_* on shutdown failure
 *
 * @note After stopping, the server can be restarted with http_server_start().
 */
esp_err_t http_server_stop(void)
{
    if (s_server == NULL) {
        ESP_LOGW(HTTP_TAG, "HTTP server not running");
        return ESP_OK;
    }
    
    esp_err_t result = httpd_stop(s_server);
    if (result == ESP_OK) {
        s_server = NULL;
        s_server_running = false;
        ESP_LOGI(HTTP_TAG, "HTTP server stopped");
    } else {
        ESP_LOGE(HTTP_TAG, "Failed to stop HTTP server: %s", esp_err_to_name(result));
    }
    
    return result;
}

/**
 * @brief Check if the HTTP server is currently running.
 *
 * Returns the current operational status of the HTTP server, indicating
 * whether it is actively accepting and processing client connections.
 *
 * @return
 *      - true if the server is running and accepting connections
 *      - false if the server is stopped or not initialized
 */
bool http_server_is_running(void)
{
    return s_server_running && (s_server != NULL);
}

/**
 * @brief Set the TCP port for the HTTP server.
 *
 * Configures the port number on which the HTTP server will listen for
 * incoming connections. This setting only takes effect when the server
 * is started; it cannot be changed while the server is running.
 *
 * @param[in] port  TCP port number (typically 80 for HTTP, 443 for HTTPS).
 *
 * @note Changes only apply to future server starts. Stop and restart
 *       the server to apply port changes.
 */
void http_server_set_port(uint16_t port)
{
    if (s_server != NULL) {
        ESP_LOGW(HTTP_TAG, "Cannot change port while server is running");
        return;
    }
    s_server_port = port;
    ESP_LOGI(HTTP_TAG, "HTTP server port set to %d", port);
}

/**
 * @brief Set the stack size for HTTP server tasks.
 *
 * Configures the amount of memory allocated for the HTTP server's task
 * stack. Larger stack sizes may be needed for complex request handlers
 * or when processing large amounts of data.
 *
 * @param[in] stack_size  Stack size in bytes for HTTP server tasks.
 *
 * @note Changes only apply to future server starts. The default stack
 *       size is usually sufficient for basic operations.
 */
void http_server_set_stack_size(size_t stack_size)
{
    if (s_server != NULL) {
        ESP_LOGW(HTTP_TAG, "Cannot change stack size while server is running");
        return;
    }
    s_stack_size = stack_size;
    ESP_LOGI(HTTP_TAG, "HTTP server stack size set to %d", stack_size);
}

/**
 * @brief Set the maximum number of simultaneous connections.
 *
 * Configures the limit for concurrent client connections that the HTTP
 * server will accept. Additional connection attempts will be rejected
 * when this limit is reached.
 *
 * @param[in] max_connections  Maximum number of simultaneous connections.
 *
 * @note Lower values reduce memory usage but may limit concurrent access.
 *       Higher values increase resource usage but improve accessibility.
 */
void http_server_set_max_connections(int max_connections)
{
    if (s_server != NULL) {
        ESP_LOGW(HTTP_TAG, "Cannot change max connections while server is running");
        return;
    }
    s_max_connections = max_connections;
    ESP_LOGI(HTTP_TAG, "HTTP server max connections set to %d", max_connections);
}
