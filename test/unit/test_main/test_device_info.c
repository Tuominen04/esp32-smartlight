/**
 * @file test_device_info.c
 * @brief Unit tests for Device Info module
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
#include "unity.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include "device_info.h"
#include "nvs_manager.h"
#include "common_defs.h"

static const char* TEST_TAG = "TEST_DEVICE";

// Test data
static const char* TEST_SSID = "TestNetwork";
static const char* TEST_PASSWORD = "TestPassword123";

/**
 * @brief Test firmware info retrieval
 */
void test_device_get_firmware_info(void)
{
    ESP_LOGI(TEST_TAG, "Testing firmware info retrieval");
    
    esp_app_desc_t* app_desc = get_firmware_info();
    TEST_ASSERT_NOT_NULL(app_desc);
    
    // Check that basic fields are populated
    TEST_ASSERT_NOT_NULL(app_desc->version);
    TEST_ASSERT_NOT_NULL(app_desc->project_name);
    TEST_ASSERT_NOT_NULL(app_desc->date);
    TEST_ASSERT_NOT_NULL(app_desc->time);
    
    // Version should not be empty
    TEST_ASSERT_GREATER_THAN(0, strlen(app_desc->version));
    
    // Project name should match our project
    ESP_LOGI(TEST_TAG, "Project name: %s", app_desc->project_name);
    ESP_LOGI(TEST_TAG, "Version: %s", app_desc->version);
    ESP_LOGI(TEST_TAG, "Date: %s", app_desc->date);
    ESP_LOGI(TEST_TAG, "Time: %s", app_desc->time);
    
    // Clean up allocated memory
    free(app_desc);
}

/**
 * @brief Test firmware info memory management
 */
void test_device_firmware_info_memory_management(void)
{
    ESP_LOGI(TEST_TAG, "Testing firmware info memory management");
    
    // Get firmware info multiple times to test memory handling
    for (int i = 0; i < 10; i++) {
        esp_app_desc_t* app_desc = get_firmware_info();
        TEST_ASSERT_NOT_NULL(app_desc);
        
        // Verify data is still valid
        TEST_ASSERT_NOT_NULL(app_desc->version);
        TEST_ASSERT_GREATER_THAN(0, strlen(app_desc->version));
        
        // Free memory
        free(app_desc);
    }
}

/**
 * @brief Test device info saving functionality
 */
void test_device_manager_save_info(void)
{
    ESP_LOGI(TEST_TAG, "Testing device manager save info");
    
    // Clear any existing device info
    nvs_manager_delete_wifi_credentials();
    
    // Save device info
    device_manager_save_device_info(TEST_SSID, TEST_PASSWORD);
    
    // Verify WiFi credentials were saved
    char retrieved_ssid[MAX_SSID_LEN] = {0};
    char retrieved_password[MAX_PASSWORD_LEN] = {0};
    
    esp_err_t result = nvs_manager_get_wifi_credentials(
        retrieved_ssid, sizeof(retrieved_ssid),
        retrieved_password, sizeof(retrieved_password)
    );
    
    TEST_ASSERT_EQUAL(ESP_OK, result);
    TEST_ASSERT_EQUAL_STRING(TEST_SSID, retrieved_ssid);
    TEST_ASSERT_EQUAL_STRING(TEST_PASSWORD, retrieved_password);
    
    // Verify device info was created
    char device_name[MAX_DEVICE_NAME_LEN] = {0};
    char device_id[MAX_DEVICE_ID_LEN] = {0};
    
    result = nvs_manager_get_device_info(
        device_name, sizeof(device_name),
        device_id, sizeof(device_id)
    );
    
    TEST_ASSERT_EQUAL(ESP_OK, result);
    TEST_ASSERT_GREATER_THAN(0, strlen(device_name));
    TEST_ASSERT_GREATER_THAN(0, strlen(device_id));
    
    // Device name should contain "ESP-C6-Light"
    TEST_ASSERT_NOT_NULL(strstr(device_name, "ESP-C6-Light"));
    
    // Device ID should be 8 characters (hex format)
    TEST_ASSERT_EQUAL(8, strlen(device_id));
}

/**
 * @brief Test device info save with existing data
 */
void test_device_manager_save_existing(void)
{
    ESP_LOGI(TEST_TAG, "Testing device manager save with existing data");
    
    // First save
    device_manager_save_device_info(TEST_SSID, TEST_PASSWORD);
    
    // Get the saved device info
    char original_name[MAX_DEVICE_NAME_LEN] = {0};
    char original_id[MAX_DEVICE_ID_LEN] = {0};
    
    nvs_manager_get_device_info(
        original_name, sizeof(original_name),
        original_id, sizeof(original_id)
    );
    
    // Save again with different credentials
    device_manager_save_device_info("NewSSID", "NewPassword");
    
    // Device info should remain the same (not overwritten)
    char new_name[MAX_DEVICE_NAME_LEN] = {0};
    char new_id[MAX_DEVICE_ID_LEN] = {0};
    
    nvs_manager_get_device_info(
        new_name, sizeof(new_name),
        new_id, sizeof(new_id)
    );
    
    TEST_ASSERT_EQUAL_STRING(original_name, new_name);
    TEST_ASSERT_EQUAL_STRING(original_id, new_id);
}

/**
 * @brief Test BLE handle setting
 */
void test_device_ble_handle_setting(void)
{
    ESP_LOGI(TEST_TAG, "Testing BLE handle setting");
    
    uint16_t test_handle = 0x1234;
    
    // Set BLE handle (should not crash)
    device_info_set_ble_handle(test_handle);
    
    // Set BLE info (should not crash)
    device_info_set_ble_info(0x01, 0x5678);
    
    TEST_ASSERT_TRUE(true); // If we get here, functions executed without crashing
}

/**
 * @brief Test invalid parameters handling
 */
void test_device_info_invalid_params(void)
{
    ESP_LOGI(TEST_TAG, "Testing invalid parameter handling");
    
    // Test NULL SSID
    device_manager_save_device_info(NULL, TEST_PASSWORD);
    // Should handle gracefully (function should not crash)
    
    // Test NULL password
    device_manager_save_device_info(TEST_SSID, NULL);
    // Should handle gracefully
    
    // Test both NULL
    device_manager_save_device_info(NULL, NULL);
    // Should handle gracefully
    
    TEST_ASSERT_TRUE(true); // If we get here, no crashes occurred
}

/**
 * @brief Test device ID format validation
 */
void test_device_id_format(void)
{
    ESP_LOGI(TEST_TAG, "Testing device ID format validation");
    
    // Clear existing data
    nvs_manager_delete_wifi_credentials();
    
    // Save device info
    device_manager_save_device_info(TEST_SSID, TEST_PASSWORD);
    
    // Get device ID
    char device_name[MAX_DEVICE_NAME_LEN] = {0};
    char device_id[MAX_DEVICE_ID_LEN] = {0};
    
    esp_err_t result = nvs_manager_get_device_info(
        device_name, sizeof(device_name),
        device_id, sizeof(device_id)
    );
    
    TEST_ASSERT_EQUAL(ESP_OK, result);
    
    // Device ID should be exactly 8 characters
    TEST_ASSERT_EQUAL(8, strlen(device_id));
    
    // Device ID should contain only hex characters
    for (int i = 0; i < 8; i++) {
        char c = device_id[i];
        bool is_hex = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
        TEST_ASSERT_TRUE(is_hex);
    }
    
    ESP_LOGI(TEST_TAG, "Generated device ID: %s", device_id);
}

/**
 * @brief Test device name format validation
 */
void test_device_name_format(void)
{
    ESP_LOGI(TEST_TAG, "Testing device name format validation");
    
    // Clear existing data
    nvs_manager_delete_wifi_credentials();
    
    // Save device info
    device_manager_save_device_info(TEST_SSID, TEST_PASSWORD);
    
    // Get device name
    char device_name[MAX_DEVICE_NAME_LEN] = {0};
    char device_id[MAX_DEVICE_ID_LEN] = {0};
    
    esp_err_t result = nvs_manager_get_device_info(
        device_name, sizeof(device_name),
        device_id, sizeof(device_id)
    );
    
    TEST_ASSERT_EQUAL(ESP_OK, result);
    
    // Device name should follow pattern: ESP-C6-Light-XXXXXXXX
    TEST_ASSERT_NOT_NULL(strstr(device_name, "ESP-C6-Light-"));
    
    // Should end with the device ID
    char expected_suffix[sizeof("ESP-C6-Light-") + MAX_DEVICE_ID_LEN + 1]; // +1 for null terminator
    snprintf(expected_suffix, sizeof(expected_suffix), "ESP-C6-Light-%s", device_id);
    TEST_ASSERT_EQUAL_STRING(expected_suffix, device_name);
    
    ESP_LOGI(TEST_TAG, "Generated device name: %s", device_name);
}

/**
 * @brief Test send device info via BLE (basic function call)
 */
void test_device_send_device_info_ble(void)
{
    ESP_LOGI(TEST_TAG, "Testing send device info via BLE");
    
    // This function requires BLE to be initialized and connected
    // For unit testing, we just verify it doesn't crash when called
    // In a real scenario, you'd mock the BLE functions
    
    send_device_info_via_ble();
    
    // If we reach here, the function didn't crash
    TEST_ASSERT_TRUE(true);
}

/**
 * @brief Test multiple device info operations
 */
void test_device_multiple_operations(void)
{
    ESP_LOGI(TEST_TAG, "Testing multiple device info operations");
    
    // Perform multiple operations in sequence
    for (int i = 0; i < 5; i++) {
        // Clear data
        nvs_manager_delete_wifi_credentials();
        
        // Save device info
        char test_ssid[32];
        snprintf(test_ssid, sizeof(test_ssid), "TestSSID_%d", i);
        device_manager_save_device_info(test_ssid, TEST_PASSWORD);
        
        // Verify it was saved
        char retrieved_ssid[MAX_SSID_LEN] = {0};
        char retrieved_password[MAX_PASSWORD_LEN] = {0};
        
        esp_err_t result = nvs_manager_get_wifi_credentials(
            retrieved_ssid, sizeof(retrieved_ssid),
            retrieved_password, sizeof(retrieved_password)
        );
        
        TEST_ASSERT_EQUAL(ESP_OK, result);
        TEST_ASSERT_EQUAL_STRING(test_ssid, retrieved_ssid);
        
        // Get firmware info
        esp_app_desc_t* app_desc = get_firmware_info();
        TEST_ASSERT_NOT_NULL(app_desc);
        free(app_desc);
    }
}
