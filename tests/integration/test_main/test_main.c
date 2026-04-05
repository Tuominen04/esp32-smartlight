/**
 * @file test_main.c
 * @brief Main test runner for ESP32 Light Controller integration tests
 *
 * Runs tests that exercise two or more components working together.
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
#include "unity.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"

static const char *TEST_TAG = "TEST_MAIN";

// device_info + nvs_manager integration tests
extern void test_device_manager_save_info(void);
extern void test_device_manager_save_existing(void);
extern void test_device_id_format(void);
extern void test_device_name_format(void);
extern void test_device_multiple_operations(void);

// wifi_manager + nvs_manager integration tests
extern void test_wifi_saved_credentials(void);
extern void test_wifi_no_saved_credentials(void);

void setUp(void) {}
void tearDown(void) {}

static void test_system_init(void)
{
  ESP_LOGI(TEST_TAG, "Initializing integration test system...");

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_create_default_wifi_sta();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

  ESP_LOGI(TEST_TAG, "Integration test system initialization complete");
}

void app_main(void)
{
  ESP_LOGI(TEST_TAG, "Starting ESP32 Light Controller Integration Tests");

  test_system_init();

  UNITY_BEGIN();

  // ===== device_info + nvs_manager =====
  ESP_LOGI(TEST_TAG, "\n=== Running device_info + nvs_manager Tests ===");
  RUN_TEST(test_device_manager_save_info);
  RUN_TEST(test_device_manager_save_existing);
  RUN_TEST(test_device_id_format);
  RUN_TEST(test_device_name_format);
  RUN_TEST(test_device_multiple_operations);

  // ===== wifi_manager + nvs_manager =====
  ESP_LOGI(TEST_TAG, "\n=== Running wifi_manager + nvs_manager Tests ===");
  RUN_TEST(test_wifi_saved_credentials);
  RUN_TEST(test_wifi_no_saved_credentials);

  UNITY_END();

  ESP_LOGI(TEST_TAG, "All integration tests completed!");
}
