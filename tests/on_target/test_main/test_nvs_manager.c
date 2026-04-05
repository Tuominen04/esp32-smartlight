/**
 * @file test_nvs_manager.c
 * @brief Unit tests for NVS Manager module
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
#include "unity.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs_manager.h"
#include "common_defs.h"

static const char* TEST_TAG = "TEST_NVS";

// Test data
static const char* TEST_SSID = "TestNetwork";
static const char* TEST_PASSWORD = "TestPassword123";
static const char* TEST_DEVICE_ID = "12345678";
static const char* TEST_DEVICE_NAME = "ESP32-Test-Device";

/**
 * @brief Test NVS initialization
 */
void test_nvs_init(void)
{
  ESP_LOGI(TEST_TAG, "Testing NVS initialization");
  
  esp_err_t result = nvs_manager_init();
  TEST_ASSERT_EQUAL(ESP_OK, result);
}

/**
 * @brief Test saving and retrieving WiFi credentials
 */
void test_nvs_wifi_credentials_save_load(void)
{
  ESP_LOGI(TEST_TAG, "Testing WiFi credentials save/load");
  
  // Save credentials
  esp_err_t result = nvs_manager_save_wifi_credentials(TEST_SSID, TEST_PASSWORD);
  TEST_ASSERT_EQUAL(ESP_OK, result);
  
  // Retrieve credentials
  char retrieved_ssid[MAX_SSID_LEN] = {0};
  char retrieved_password[MAX_PASSWORD_LEN] = {0};
  
  result = nvs_manager_get_wifi_credentials(
    retrieved_ssid, sizeof(retrieved_ssid),
    retrieved_password, sizeof(retrieved_password)
  );
  
  TEST_ASSERT_EQUAL(ESP_OK, result);
  TEST_ASSERT_EQUAL_STRING(TEST_SSID, retrieved_ssid);
  TEST_ASSERT_EQUAL_STRING(TEST_PASSWORD, retrieved_password);
}

/**
 * @brief Test saving and retrieving device information
 */
void test_nvs_device_info_save_load(void)
{
  ESP_LOGI(TEST_TAG, "Testing device info save/load");
  
  // Save device info
  esp_err_t result = nvs_manager_save_device_info(TEST_DEVICE_ID, TEST_DEVICE_NAME);
  TEST_ASSERT_EQUAL(ESP_OK, result);
  
  // Retrieve device info
  char retrieved_name[MAX_DEVICE_NAME_LEN] = {0};
  char retrieved_id[MAX_DEVICE_ID_LEN] = {0};
  
  result = nvs_manager_get_device_info(
    retrieved_name, sizeof(retrieved_name),
    retrieved_id, sizeof(retrieved_id)
  );
  
  TEST_ASSERT_EQUAL(ESP_OK, result);
  TEST_ASSERT_EQUAL_STRING(TEST_DEVICE_NAME, retrieved_name);
  TEST_ASSERT_EQUAL_STRING(TEST_DEVICE_ID, retrieved_id);
}

/**
 * @brief Test deleting WiFi credentials
 */
void test_nvs_wifi_credentials_delete(void)
{
  ESP_LOGI(TEST_TAG, "Testing WiFi credentials deletion");
  
  // First save some credentials
  esp_err_t result = nvs_manager_save_wifi_credentials(TEST_SSID, TEST_PASSWORD);
  TEST_ASSERT_EQUAL(ESP_OK, result);
  
  // Delete credentials
  result = nvs_manager_delete_wifi_credentials();
  TEST_ASSERT_EQUAL(ESP_OK, result);
  
  // Try to retrieve - should fail
  char retrieved_ssid[MAX_SSID_LEN] = {0};
  char retrieved_password[MAX_PASSWORD_LEN] = {0};
  
  result = nvs_manager_get_wifi_credentials(
    retrieved_ssid, sizeof(retrieved_ssid),
    retrieved_password, sizeof(retrieved_password)
  );
  
  // Should return not found error
  TEST_ASSERT_NOT_EQUAL(ESP_OK, result);
}

/**
 * @brief Test handling of invalid parameters
 */
void test_nvs_invalid_params(void)
{
  ESP_LOGI(TEST_TAG, "Testing invalid parameter handling");
  
  // Test NULL parameters
  esp_err_t result = nvs_manager_save_wifi_credentials(NULL, TEST_PASSWORD);
  TEST_ASSERT_NOT_EQUAL(ESP_OK, result);
  
  result = nvs_manager_save_wifi_credentials(TEST_SSID, NULL);
  TEST_ASSERT_NOT_EQUAL(ESP_OK, result);
  
  // Test NULL output parameters
  result = nvs_manager_get_wifi_credentials(NULL, 32, NULL, 32);
  TEST_ASSERT_NOT_EQUAL(ESP_OK, result);
}

/**
 * @brief Test buffer size handling
 */
void test_nvs_buffer_sizes(void)
{
  ESP_LOGI(TEST_TAG, "Testing buffer size handling");
  
  // Save test data
  nvs_manager_save_wifi_credentials(TEST_SSID, TEST_PASSWORD);
  
  // Test with too small buffer
  char small_buffer[5] = {0};
  char normal_buffer[MAX_PASSWORD_LEN] = {0};
  
  esp_err_t result = nvs_manager_get_wifi_credentials(
      small_buffer, sizeof(small_buffer),
      normal_buffer, sizeof(normal_buffer)
  );
  
  // Should handle gracefully (ESP-IDF NVS handles this)
  // The exact behavior depends on ESP-IDF version
  ESP_LOGI(TEST_TAG, "Small buffer test result: %s", esp_err_to_name(result));
}
