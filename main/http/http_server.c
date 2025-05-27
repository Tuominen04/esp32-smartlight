#include "http_server.h"
#include <string.h>
#include <stdio.h>
#include "esp_log.h"
#include "esp_http_server.h"

// Include other modules for handlers
#include "../gpio/gpio_control.h"
#include "../ota/ota_manager.h"
#include "../common/credentials.h"
#include "../device/device_info.h"

static const char *HTTP_TAG = "HTTP_SERVER";

// Private variables
static httpd_handle_t s_server = NULL;
static bool s_server_running = false;
static uint16_t s_server_port = 80;
static size_t s_stack_size = 8192;
static int s_max_connections = 3;

// Light control handlers
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

// Initialize HTTP server (but don't start it)
esp_err_t http_server_init(void)
{
    if (s_server != NULL) {
        ESP_LOGW(HTTP_TAG, "HTTP server already initialized");
        return ESP_OK;
    }
    
    ESP_LOGI(HTTP_TAG, "HTTP server initialized");
    return ESP_OK;
}

// Start the HTTP server
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

// Stop the HTTP server
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

// Check if server is running
bool http_server_is_running(void)
{
    return s_server_running && (s_server != NULL);
}

// Configuration setters
void http_server_set_port(uint16_t port)
{
    if (s_server != NULL) {
        ESP_LOGW(HTTP_TAG, "Cannot change port while server is running");
        return;
    }
    s_server_port = port;
    ESP_LOGI(HTTP_TAG, "HTTP server port set to %d", port);
}

void http_server_set_stack_size(size_t stack_size)
{
    if (s_server != NULL) {
        ESP_LOGW(HTTP_TAG, "Cannot change stack size while server is running");
        return;
    }
    s_stack_size = stack_size;
    ESP_LOGI(HTTP_TAG, "HTTP server stack size set to %d", stack_size);
}

void http_server_set_max_connections(int max_connections)
{
    if (s_server != NULL) {
        ESP_LOGW(HTTP_TAG, "Cannot change max connections while server is running");
        return;
    }
    s_max_connections = max_connections;
    ESP_LOGI(HTTP_TAG, "HTTP server max connections set to %d", max_connections);
}

// Register light control routes
esp_err_t http_server_register_light_routes(void)
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

// Register OTA routes
esp_err_t http_server_register_ota_routes(void)
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

// Register all routes
esp_err_t http_server_register_all_routes(void)
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