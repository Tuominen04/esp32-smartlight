/**
 * @file main.c
 * @brief Main application entry point for ESP32 Light Client
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
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

// Module includes
#include "gpio_control.h"
#include "device_info.h"
#include "wifi_manager.h"
#include "ble_manager.h"
#include "ota_manager.h"
#include "http_server.h"
#include "nvs_manager.h"
#include "common_defs.h"

static const char *MAIN_TAG = "MAIN";

/**
 * @brief Callback function executed when WiFi connection is established.
 *
 * Starts the HTTP server to enable remote control and OTA functionality
 * once the device has network connectivity.
 */
void on_wifi_connected(void) {
  ESP_LOGI(MAIN_TAG, "WiFi connected - starting HTTP server");
  esp_err_t result = http_server_start();
  if (result != ESP_OK) {
    ESP_LOGE(MAIN_TAG, "Failed to start HTTP server: %s", esp_err_to_name(result));
  }
}

/**
 * @brief Callback function executed when WiFi connection is lost.
 *
 * Stops the HTTP server to prevent access attempts when the device
 * loses network connectivity.
 */
void on_wifi_disconnected(void) {
  ESP_LOGI(MAIN_TAG, "WiFi disconnected - stopping HTTP server");
  http_server_stop();
}

/**
 * @brief Main application entry point.
 *
 * Initializes all system modules, attempts to connect using saved WiFi
 * credentials, starts background tasks for BLE configuration and WiFi
 * credential handling, and sets up the complete light control system.
 *
 * Initialization sequence:
 * 1. Core systems (NVS, firmware info)
 * 2. Hardware modules (GPIO, BLE, WiFi, OTA, HTTP)
 * 3. WiFi connection attempt with saved credentials
 * 4. Background task creation for ongoing operations
 *
 * @note This function never returns; the system runs via FreeRTOS tasks.
 */
void app_main(void)
{
  ESP_LOGI(MAIN_TAG, "Starting Light Client Application");
  
  // Initialize core systems first
  ESP_ERROR_CHECK(nvs_manager_init());
  
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
  char saved_ssid[MAX_SSID_BUF_LEN] = {0};
  char saved_password[MAX_PASSWORD_LEN] = {0};
  
  if (wifi_manager_get_saved_credentials(saved_ssid, sizeof(saved_ssid), saved_password, sizeof(saved_password)) == ESP_OK) {
    ESP_LOGI(MAIN_TAG, "Found saved WiFi credentials");
    wifi_connect(saved_ssid, saved_password);
  } else {
    ESP_LOGI(MAIN_TAG, "No saved WiFi credentials found - waiting for BLE configuration");
  }

  // Start background tasks
  ESP_LOGI(MAIN_TAG, "Starting background tasks...");
  
  if (xTaskCreate(
      wifi_manager_handle_new_credentials_task,
      "wifi_cred_handler",
      WIFI_TASK_STACK_SIZE,
      NULL,
      WIFI_TASK_PRIORITY,
      NULL
    ) != pdPASS) {
    ESP_LOGE(MAIN_TAG, "Failed to create WiFi credential handler task");
  }

  if (xTaskCreate(
      ble_manager_handle_device_info_confirmation,
      "conf_handler",
      BLE_TASK_STACK_SIZE,
      NULL,
      BLE_TASK_PRIORITY,
      NULL
    ) != pdPASS) {
    ESP_LOGE(MAIN_TAG, "Failed to create BLE confirmation handler task");
  }
  
  ESP_LOGI(MAIN_TAG, "Light Client Application started successfully");
}