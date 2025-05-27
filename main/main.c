#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

// Module includes
#include "gpio/gpio_control.h"
#include "device/device_info.h"
#include "wifi/wifi_manager.h"
#include "ble/ble_manager.h"
#include "ota/ota_manager.h"
#include "http/http_server.h"
#include "storage/nvs_manager.h"
#include "common/common_defs.h"

static const char *MAIN_TAG = "MAIN";

// Callback for when WiFi connects
void on_wifi_connected(void) {
    ESP_LOGI(MAIN_TAG, "WiFi connected - starting HTTP server");
    esp_err_t result = http_server_start();
    if (result != ESP_OK) {
        ESP_LOGE(MAIN_TAG, "Failed to start HTTP server: %s", esp_err_to_name(result));
    }
}

void on_wifi_disconnected(void) {
    ESP_LOGI(MAIN_TAG, "WiFi disconnected - stopping HTTP server");
    http_server_stop();
}

/************************ Main *************************/
void app_main(void)
{
    ESP_LOGI(MAIN_TAG, "Starting Light Client Application");
    

    // Initialize core systems first
    ESP_ERROR_CHECK(nvs_manager_init());
    //ESP_ERROR_CHECK(event_handlers_init());
    
    // Get firmware info
    get_firmware_info();    

    // Initialize all modules
    ESP_LOGI(MAIN_TAG, "Initializing modules...");
    ESP_ERROR_CHECK(gpio_control_init());
    ESP_ERROR_CHECK(ble_manager_init());
    ESP_ERROR_CHECK(wifi_manager_init());
    ESP_ERROR_CHECK(ota_manager_init());
    ESP_ERROR_CHECK(http_server_init());

    // Set WiFi callbacks
    wifi_manager_set_callbacks(on_wifi_connected, on_wifi_disconnected);

    // Try to load saved WiFi credentials
    char saved_ssid[MAX_SSID_LEN] = {0};
    char saved_password[MAX_PASSWORD_LEN] = {0};
    
    if (wifi_manager_get_saved_credentials(saved_ssid, sizeof(saved_ssid), saved_password, sizeof(saved_password)) == ESP_OK) {
        ESP_LOGI(MAIN_TAG, "Found saved WiFi credentials for %s", saved_ssid);
        wifi_connect(saved_ssid, saved_password);
    } else {
        ESP_LOGI(MAIN_TAG, "No saved WiFi credentials found - waiting for BLE configuration");
    }

    // Start background tasks
    ESP_LOGI(MAIN_TAG, "Starting background tasks...");
    
    xTaskCreate(
        wifi_manager_handle_new_credentials_task,
        "wifi_cred_handler",
        WIFI_TASK_STACK_SIZE,
        NULL,
        WIFI_TASK_PRIORITY,
        NULL
    );

    xTaskCreate(
        ble_manager_handle_device_info_confirmation,
        "conf_handler",
        BLE_TASK_STACK_SIZE,
        NULL,
        BLE_TASK_PRIORITY,
        NULL
    );
    
    ESP_LOGI(MAIN_TAG, "Light Client Application started successfully");
}