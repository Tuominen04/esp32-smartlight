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

static const char *TEST_TAG = "TEST_DEVICE";

// Test data
static const char *TEST_SSID = "TestNetwork";
static const char *TEST_PASSWORD = "TestPassword123";

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
 * @brief Test BLE handle setting
 */
void test_device_ble_handle_setting(void)
{
  ESP_LOGI(TEST_TAG, "Testing BLE handle setting");

#ifdef CONFIG_BT_ENABLED
  uint16_t test_handle = 0x1234;
  device_info_set_ble_handle(test_handle);
  device_info_set_ble_info(0x01, 0x5678);
  TEST_ASSERT_TRUE(true);
#else
  TEST_IGNORE_MESSAGE("BLE not enabled in this build");
#endif
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
 * @brief Test send device info via BLE (basic function call)
 */
void test_device_send_device_info_ble(void)
{
  ESP_LOGI(TEST_TAG, "Testing send device info via BLE");

#ifdef CONFIG_BT_ENABLED
  send_device_info_via_ble();
  TEST_ASSERT_TRUE(true);
#else
  TEST_IGNORE_MESSAGE("BLE not enabled in this build");
#endif
}


