/**
 * @file test_main.c
 * @brief Main test runner for ESP32 Light Controller
 *
 * This file contains the main test application that runs all unit tests
 * for the light controller project.
 *
 * Copyright (c) 2025 Arttu Tuominen. All rights reserved.
 *
 * This software is licensed under commercial terms.
 * See LICENSE file for complete license terms.
 *
 * NOTICE: This file contains proprietary information. Unauthorized
 * distribution or use is strictly prohibited.
 */

#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"

static const char* TEST_TAG = "TEST_MAIN";

// Test Device function declarations
extern void test_device_get_firmware_info(void);
extern void test_device_firmware_info_memory_management(void);
extern void test_device_manager_save_info(void);
extern void test_device_manager_save_existing(void);
extern void test_device_ble_handle_setting(void);
extern void test_device_info_invalid_params(void);
extern void test_device_id_format(void);
extern void test_device_name_format(void);
extern void test_device_send_device_info_ble(void);
extern void test_device_multiple_operations(void);

// Test GPIO function declarations
extern void test_gpio_init(void);
extern void test_gpio_light_state_control(void);
extern void test_gpio_light_toggle(void);
extern void test_gpio_rapid_state_changes(void);
extern void test_gpio_state_consistency(void);
extern void test_gpio_initial_state(void);
extern void test_gpio_toggle_performance(void);

// Test NVS function declarations
extern void test_nvs_init(void);
extern void test_nvs_wifi_credentials_save_load(void);
extern void test_nvs_device_info_save_load(void);
extern void test_nvs_wifi_credentials_delete(void);
extern void test_nvs_invalid_params(void);
extern void test_nvs_buffer_sizes(void);

// Test WiFi function declarations
extern void test_wifi_manager_init(void);
extern void test_wifi_initial_connection_state(void);
extern void test_wifi_set_new_credentials(void);
extern void test_wifi_set_invalid_credentials(void);
extern void test_wifi_credential_length_limits(void);
extern void test_wifi_json_credential_format(void);
extern void test_wifi_saved_credentials(void);
extern void test_wifi_no_saved_credentials(void);
extern void test_wifi_disconnect(void);
extern void test_wifi_callback_setting(void);
extern void test_wifi_event_group_access(void);

/**
 * @brief Setup function called before each test
 */
void setUp(void)
{
    // Called before each test
}

/**
 * @brief Teardown function called after each test
 */
void tearDown(void)
{
    // Called after each test
}

/**
 * @brief Initialize system components needed for testing
 */
static void test_system_init(void)
{
    ESP_LOGI(TEST_TAG, "Initializing test system...");

    // Initialize NVS for tests that need it
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize network interface for WiFi tests
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize WiFi for MAC address access
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_LOGI(TEST_TAG, "Test system initialization complete");
}

/**
 * @brief Main test application
 */
void app_main(void)
{
    ESP_LOGI(TEST_TAG, "Starting ESP32 Light Controller Unit Tests");

    // Initialize system components
    test_system_init();

    // Initialize Unity test framework
    UNITY_BEGIN();

    // ===== Device Info Tests =====
    ESP_LOGI(TEST_TAG, "\n=== Running Device Info Tests ===");
    RUN_TEST(test_device_get_firmware_info);
    RUN_TEST(test_device_firmware_info_memory_management);
    RUN_TEST(test_device_manager_save_info);
    RUN_TEST(test_device_manager_save_existing);
    RUN_TEST(test_device_ble_handle_setting);
    RUN_TEST(test_device_info_invalid_params);
    RUN_TEST(test_device_id_format);
    RUN_TEST(test_device_name_format);
    RUN_TEST(test_device_send_device_info_ble);
    RUN_TEST(test_device_multiple_operations);

    // ===== GPIO Control Tests =====
    ESP_LOGI(TEST_TAG, "\n=== Running GPIO Control Tests ===");
    RUN_TEST(test_gpio_init);
    RUN_TEST(test_gpio_light_state_control);
    RUN_TEST(test_gpio_light_toggle);
    RUN_TEST(test_gpio_rapid_state_changes);
    RUN_TEST(test_gpio_state_consistency);
    RUN_TEST(test_gpio_initial_state);
    RUN_TEST(test_gpio_toggle_performance);

    // ===== NVS Manager Tests =====
    ESP_LOGI(TEST_TAG, "\n=== Running NVS Manager Tests ===");
    RUN_TEST(test_nvs_init);
    RUN_TEST(test_nvs_wifi_credentials_save_load);
    RUN_TEST(test_nvs_device_info_save_load);
    RUN_TEST(test_nvs_wifi_credentials_delete);
    RUN_TEST(test_nvs_invalid_params);
    RUN_TEST(test_nvs_buffer_sizes);

    // ===== WiFi Tests =====
    ESP_LOGI(TEST_TAG, "\n=== Running WiFI Tests ===");
    RUN_TEST(test_wifi_manager_init);
    RUN_TEST(test_wifi_initial_connection_state);
    RUN_TEST(test_wifi_set_new_credentials);
    RUN_TEST(test_wifi_set_invalid_credentials);
    RUN_TEST(test_wifi_credential_length_limits);
    RUN_TEST(test_wifi_json_credential_format);
    RUN_TEST(test_wifi_saved_credentials);
    RUN_TEST(test_wifi_no_saved_credentials);
    RUN_TEST(test_wifi_disconnect);
    RUN_TEST(test_wifi_callback_setting);
    RUN_TEST(test_wifi_event_group_access);

    // Finish Unity testing
    UNITY_END();

    ESP_LOGI(TEST_TAG, "All tests completed!");
}
