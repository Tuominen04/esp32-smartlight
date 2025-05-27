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
#include "device/device_info.h"
#include "storage/nvs_manager.h"

static const char *WIFI_TAG = "WIFI_MANAGER";

// Private variables
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
static bool s_wifi_initialized = false;
static bool s_wifi_connected = false;

// WiFi credentials buffer management
static char s_wifi_credentials_buffer[256] = {0};
static size_t s_wifi_credentials_len = 0;
static bool s_new_wifi_credentials = false;

// WiFi configuration
static wifi_config_t s_wifi_config = {
    .sta = {
        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
    },
};

// Callback functions
static wifi_connected_cb_t s_connected_callback = NULL;
static wifi_disconnected_cb_t s_disconnected_callback = NULL;

// Forward declarations
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

// Initialize WiFi manager
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

    // Initialize network interface
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register event handlers
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                       ESP_EVENT_ANY_ID,
                                                       &wifi_event_handler,
                                                       NULL,
                                                       &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                       IP_EVENT_STA_GOT_IP,
                                                       &wifi_event_handler,
                                                       NULL,
                                                       &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    
    s_wifi_initialized = true;
    ESP_LOGI(WIFI_TAG, "WiFi manager initialized successfully");
    
    return ESP_OK;
}

// Connect to WiFi with credentials
esp_err_t wifi_manager_connect(const char *ssid, const char *password)
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

// Check if WiFi is connected
bool wifi_manager_is_connected(void)
{
    return s_wifi_connected;
}

// Disconnect from WiFi
esp_err_t wifi_manager_disconnect(void)
{
    if (!s_wifi_initialized) {
        return ESP_FAIL;
    }

    ESP_LOGI(WIFI_TAG, "Disconnecting from WiFi");
    s_wifi_connected = false;
    return esp_wifi_disconnect();
}

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

// Set WiFi credentials from JSON
void wifi_manager_set_new_credentials(const char *json_credentials)
{
    if (!json_credentials) {
        ESP_LOGE(WIFI_TAG, "NULL credentials provided");
        return;
    }

    size_t len = strlen(json_credentials);
    if (len >= sizeof(s_wifi_credentials_buffer)) {
        ESP_LOGE(WIFI_TAG, "Credentials too long: %d bytes", len);
        return;
    }

    // Copy credentials to buffer
    memset(s_wifi_credentials_buffer, 0, sizeof(s_wifi_credentials_buffer));
    strncpy(s_wifi_credentials_buffer, json_credentials, sizeof(s_wifi_credentials_buffer) - 1);
    s_wifi_credentials_len = len;
    s_new_wifi_credentials = true;

    ESP_LOGI(WIFI_TAG, "New WiFi credentials received");
}

// Check if new credentials are available
bool wifi_manager_has_new_credentials(void)
{
    return s_new_wifi_credentials;
}

// Task to handle new WiFi credentials
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

// Set callback functions
void wifi_manager_set_callbacks(wifi_connected_cb_t on_connect, wifi_disconnected_cb_t on_disconnect)
{
    s_connected_callback = on_connect;
    s_disconnected_callback = on_disconnect;
}

// Get event group handle
EventGroupHandle_t wifi_manager_get_event_group(void)
{
    return s_wifi_event_group;
}

// WiFi event handler
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

// Legacy compatibility functions
void wifi_init(void)
{
    wifi_manager_init();
}

void wifi_init_sta(const char *ssid, const char *pass)
{
    wifi_manager_init();
    wifi_manager_connect(ssid, pass);
    
    // Wait for connection
    if (s_wifi_event_group) {
        EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                pdFALSE, pdFALSE, portMAX_DELAY);

        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(WIFI_TAG, "Connected to WiFi network");
        } else if (bits & WIFI_FAIL_BIT) {
            ESP_LOGI(WIFI_TAG, "Failed to connect to WiFi network: %s", ssid);
        }
    }
}

void wifi_connect(const char *ssid, const char *pass)
{
    wifi_manager_connect(ssid, pass);
}