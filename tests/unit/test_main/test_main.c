/**
 * @file test_main.c
 * @brief Unit test runner for ESP32 Light Controller
 *
 * Runs all unit tests against the linux target. No hardware initialisation
 * is performed — these tests exercise pure logic only.
 *
 * Exit behaviour:
 *   - Exits with code 0 when all tests pass.
 *   - Exits with code 1 when any test fails, so CI jobs detect failures.
 *
 * Copyright (c) 2025 Arttu Tuominen. All rights reserved.
 *
 * This software is licensed under commercial terms.
 * See LICENSE file for complete license terms.
 *
 * NOTICE: This file contains proprietary information. Unauthorized
 * distribution or use is strictly prohibited.
 */

#include <stdlib.h>
#include "unity.h"
#include "esp_log.h"

static const char *TEST_TAG = "UNIT_TEST";

/* =========================================================================
 * test_common_defs.c declarations
 * ========================================================================= */
extern void test_ssid_len_satisfies_wifi_standard(void);
extern void test_password_len_satisfies_wpa2_max(void);
extern void test_device_name_len_fits_default_name(void);
extern void test_device_id_len_is_sufficient(void);
extern void test_nvs_wifi_ssid_key_is_valid(void);
extern void test_nvs_wifi_pass_key_is_valid(void);
extern void test_nvs_device_name_key_is_valid(void);
extern void test_nvs_device_id_key_is_valid(void);
extern void test_default_device_name_is_not_empty(void);
extern void test_device_number_is_positive(void);

/* =========================================================================
 * test_credential_validation.c declarations
 * ========================================================================= */
extern void test_valid_credentials_accepted(void);
extern void test_single_char_ssid_accepted(void);
extern void test_ssid_at_max_length_fits_buffer(void);
extern void test_password_at_max_length_fits_buffer(void);
extern void test_max_credentials_fit_json_buffer(void);
extern void test_empty_ssid_rejected(void);
extern void test_empty_password_rejected(void);
extern void test_null_ssid_rejected(void);
extern void test_null_password_rejected(void);
extern void test_ssid_over_max_length_does_not_fit(void);
extern void test_password_over_max_length_does_not_fit(void);

/* =========================================================================
 * Unity callbacks
 * ========================================================================= */

void setUp(void)
{
}

void tearDown(void)
{
}

/* =========================================================================
 * Entry point
 * ========================================================================= */

void app_main(void)
{
  ESP_LOGI(TEST_TAG, "Starting unit tests (linux target)");

  UNITY_BEGIN();

  /* --- common_defs constants --- */
  ESP_LOGI(TEST_TAG, "=== common_defs tests ===");
  RUN_TEST(test_ssid_len_satisfies_wifi_standard);
  RUN_TEST(test_password_len_satisfies_wpa2_max);
  RUN_TEST(test_device_name_len_fits_default_name);
  RUN_TEST(test_device_id_len_is_sufficient);
  RUN_TEST(test_nvs_wifi_ssid_key_is_valid);
  RUN_TEST(test_nvs_wifi_pass_key_is_valid);
  RUN_TEST(test_nvs_device_name_key_is_valid);
  RUN_TEST(test_nvs_device_id_key_is_valid);
  RUN_TEST(test_default_device_name_is_not_empty);
  RUN_TEST(test_device_number_is_positive);

  /* --- credential validation --- */
  ESP_LOGI(TEST_TAG, "=== credential validation tests ===");
  RUN_TEST(test_valid_credentials_accepted);
  RUN_TEST(test_single_char_ssid_accepted);
  RUN_TEST(test_ssid_at_max_length_fits_buffer);
  RUN_TEST(test_password_at_max_length_fits_buffer);
  RUN_TEST(test_max_credentials_fit_json_buffer);
  RUN_TEST(test_empty_ssid_rejected);
  RUN_TEST(test_empty_password_rejected);
  RUN_TEST(test_null_ssid_rejected);
  RUN_TEST(test_null_password_rejected);
  RUN_TEST(test_ssid_over_max_length_does_not_fit);
  RUN_TEST(test_password_over_max_length_does_not_fit);

  int failures = UNITY_END();

  ESP_LOGI(TEST_TAG, "Unit tests complete");

  exit(failures > 0 ? 1 : 0);
}
