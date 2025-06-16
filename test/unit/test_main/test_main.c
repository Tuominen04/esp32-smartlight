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
 * 
 * @note file contains the main test application that runs all unit tests
 * for the light controller project.
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

#define MAX_TESTS 50

// Structure to track test results
typedef struct {
    char name[64];
    bool passed;
    char failure_message[256];
} test_result_t;

static const char* TEST_TAG = "TEST_MAIN";
static int test_count = 0;
static test_result_t test_results[MAX_TESTS]; // Array to store test results


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
 * @brief Print detailed test results including failure information
 */
void print_detailed_test_summary(void)
{
    int passed_count = 0;
    int failed_count = 0;
    
    // Count results
    for (int i = 0; i < test_count; i++) {
        if (test_results[i].passed) {
            passed_count++;
        } else {
            failed_count++;
        }
    }
    
    printf("\n");
    printf("===============================================\n");
    printf("               TEST RESULTS\n");
    printf("===============================================\n");
    
    // Summary
    printf("SUMMARY:\n");
    printf("========\n");
    printf("Total Tests: %d\n", test_count);
    printf("Passed:      %d (%.1f%%)\n", passed_count, 
           test_count > 0 ? (passed_count * 100.0f / test_count) : 0);
    printf("Failed:      %d (%.1f%%)\n", failed_count,
           test_count > 0 ? (failed_count * 100.0f / test_count) : 0);
    printf("===============================================\n");

    // Print failed tests first if any
    if (failed_count > 0) {
        printf("FAILED TESTS:\n");
        printf("=============\n");
        int fail_number = 1;
        for (int i = 0; i < test_count; i++) {
            if (!test_results[i].passed) {
                printf("%d. %s\n", fail_number++, test_results[i].name);
                printf("\n");
            }
        }
        printf("==============================================\n");
    }
}

/**
 * @brief Custom test runner that captures Unity output and tracks results
 */
void run_test_with_detailed_tracking(UnityTestFunction test, const char* name, const UNITY_LINE_TYPE line)
{
    printf("\n🧪 TESTING: %s\n", name);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    // Add test to tracking array
    if (test_count < MAX_TESTS) {
        strncpy(test_results[test_count].name, name, sizeof(test_results[test_count].name) - 1);
        test_results[test_count].name[sizeof(test_results[test_count].name) - 1] = '\0';
        test_results[test_count].passed = true; // Assume pass initially
        test_results[test_count].failure_message[0] = '\0';
        test_count++;
    }
    
    // Store current test state
    UNITY_UINT failures_before = Unity.TestFailures;
    
    // Run the test - Unity will handle the assertions
    UnityDefaultTestRun(test, name, line);

    // Check if test passed or failed
    UNITY_UINT failures_after = Unity.TestFailures;
    bool test_passed = (failures_after == failures_before);
    
    // Update our tracking
    if (test_count > 0) {
        test_results[test_count - 1].passed = test_passed;
    }
    
    if (test_passed) {
        printf("🟢 RESULT: PASSED\n");
    } else {
        printf("🔴 RESULT: FAILED\n");
        // Print Unity's failure details immediately
        if (Unity.CurrentTestFailed) {
            printf("   Unity Details: %s\n", Unity.CurrentTestName);
        }
    }
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    fflush(stdout);
}

/**
 * @brief Macro to run a test with detailed tracking
 *
 * This macro wraps the Unity test function to capture its name and line number,
 * and track the result in our custom test results array.
 */
#define RUN_CUSTOM_TEST(func) run_test_with_detailed_tracking(func, #func, __LINE__)

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
    RUN_CUSTOM_TEST(test_device_get_firmware_info);
    RUN_CUSTOM_TEST(test_device_firmware_info_memory_management);
    RUN_CUSTOM_TEST(test_device_manager_save_info);
    RUN_CUSTOM_TEST(test_device_manager_save_existing);
    RUN_CUSTOM_TEST(test_device_ble_handle_setting);
    RUN_CUSTOM_TEST(test_device_info_invalid_params);
    RUN_CUSTOM_TEST(test_device_id_format);
    RUN_CUSTOM_TEST(test_device_name_format);
    RUN_CUSTOM_TEST(test_device_send_device_info_ble);
    RUN_CUSTOM_TEST(test_device_multiple_operations);

    // ===== GPIO Control Tests =====
    ESP_LOGI(TEST_TAG, "\n=== Running GPIO Control Tests ===");
    RUN_CUSTOM_TEST(test_gpio_init);
    RUN_CUSTOM_TEST(test_gpio_light_state_control);
    RUN_CUSTOM_TEST(test_gpio_light_toggle);
    RUN_CUSTOM_TEST(test_gpio_rapid_state_changes);
    RUN_CUSTOM_TEST(test_gpio_state_consistency);
    RUN_CUSTOM_TEST(test_gpio_initial_state);
    RUN_CUSTOM_TEST(test_gpio_toggle_performance);

    // ===== NVS Manager Tests =====
    ESP_LOGI(TEST_TAG, "\n=== Running NVS Manager Tests ===");
    RUN_CUSTOM_TEST(test_nvs_init);
    RUN_CUSTOM_TEST(test_nvs_wifi_credentials_save_load);
    RUN_CUSTOM_TEST(test_nvs_device_info_save_load);
    RUN_CUSTOM_TEST(test_nvs_wifi_credentials_delete);
    RUN_CUSTOM_TEST(test_nvs_invalid_params);
    RUN_CUSTOM_TEST(test_nvs_buffer_sizes);

    // ===== WiFi Tests =====
    ESP_LOGI(TEST_TAG, "\n=== Running WiFI Tests ===");
    RUN_CUSTOM_TEST(test_wifi_manager_init);
    RUN_CUSTOM_TEST(test_wifi_initial_connection_state);
    RUN_CUSTOM_TEST(test_wifi_set_new_credentials);
    RUN_CUSTOM_TEST(test_wifi_set_invalid_credentials);
    RUN_CUSTOM_TEST(test_wifi_credential_length_limits);
    RUN_CUSTOM_TEST(test_wifi_json_credential_format);
    RUN_CUSTOM_TEST(test_wifi_saved_credentials);
    RUN_CUSTOM_TEST(test_wifi_no_saved_credentials);
    RUN_CUSTOM_TEST(test_wifi_disconnect);
    RUN_CUSTOM_TEST(test_wifi_callback_setting);
    RUN_CUSTOM_TEST(test_wifi_event_group_access);

    print_detailed_test_summary();

    // Finish Unity testing
    UNITY_END();
    
    ESP_LOGI(TEST_TAG, "All tests completed!");
}
