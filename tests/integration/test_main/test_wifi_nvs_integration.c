/**
 * @file test_wifi_nvs_integration.c
 * @brief Integration tests for wifi_manager + nvs_manager interaction
 *
 * These tests verify that wifi_manager correctly reads credentials
 * that were stored via the nvs_manager component.
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
#include "wifi_manager.h"
#include "nvs_manager.h"
#include "common_defs.h"

static const char *TEST_TAG = "TEST_WIFI_NVS";

static const char *TEST_SSID     = "TestNetwork";
static const char *TEST_PASSWORD = "TestPassword123";

/**
 * @brief Test that wifi_manager_get_saved_credentials reads what nvs_manager stored
 */
void test_wifi_saved_credentials(void)
{
  ESP_LOGI(TEST_TAG, "Testing saved credentials cross-component retrieval");

  esp_err_t res = wifi_manager_init();
  if (res != ESP_OK) {
    TEST_FAIL_MESSAGE("WiFi manager initialization failed");
  }

  esp_err_t result = nvs_manager_save_wifi_credentials(TEST_SSID, TEST_PASSWORD);
  TEST_ASSERT_EQUAL(ESP_OK, result);

  char retrieved_ssid[MAX_SSID_BUF_LEN]         = {0};
  char retrieved_password[MAX_PASSWORD_LEN] = {0};

  result = wifi_manager_get_saved_credentials(
    retrieved_ssid, sizeof(retrieved_ssid),
    retrieved_password, sizeof(retrieved_password));

  TEST_ASSERT_EQUAL(ESP_OK, result);
  TEST_ASSERT_EQUAL_STRING(TEST_SSID, retrieved_ssid);
  TEST_ASSERT_EQUAL_STRING(TEST_PASSWORD, retrieved_password);
}

/**
 * @brief Test that wifi_manager_get_saved_credentials fails when nvs_manager
 *        has no stored credentials
 */
void test_wifi_no_saved_credentials(void)
{
  ESP_LOGI(TEST_TAG, "Testing wifi_manager behaviour with no NVS credentials");

  esp_err_t res = wifi_manager_init();
  if (res != ESP_OK) {
    TEST_FAIL_MESSAGE("WiFi manager initialization failed");
  }

  nvs_manager_delete_wifi_credentials();

  char retrieved_ssid[MAX_SSID_BUF_LEN]         = {0};
  char retrieved_password[MAX_PASSWORD_LEN] = {0};

  esp_err_t result = wifi_manager_get_saved_credentials(
    retrieved_ssid, sizeof(retrieved_ssid),
    retrieved_password, sizeof(retrieved_password));

  TEST_ASSERT_NOT_EQUAL(ESP_OK, result);
}
