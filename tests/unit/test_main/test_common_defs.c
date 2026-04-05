/**
 * @file test_common_defs.c
 * @brief Unit tests for common_defs.h constants and configuration invariants
 *
 * Verifies that buffer sizes, NVS key names, and device configuration
 * constants satisfy the requirements expected by the rest of the firmware
 * (WiFi protocol limits, NVS key constraints, etc.).
 *
 * These tests run on the linux target in CI without physical hardware.
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
#include "common_defs.h"

/* NVS key length limit imposed by the NVS API (including null terminator). */
#define NVS_KEY_MAX_LEN 16

/* =========================================================================
 * Buffer size tests
 * ========================================================================= */

/**
 * @brief MAX_SSID_LEN must accommodate the full 32-byte WiFi SSID limit.
 *
 * IEEE 802.11 allows SSIDs up to 32 bytes. MAX_SSID_LEN must include space
 * for the null terminator, so the minimum valid value is 33.
 */
void test_ssid_len_satisfies_wifi_standard(void)
{
  /* 32 bytes payload + 1 null terminator */
  TEST_ASSERT_GREATER_OR_EQUAL(33, MAX_SSID_LEN);
}

/**
 * @brief MAX_PASSWORD_LEN must accommodate the WPA2 maximum passphrase length.
 *
 * WPA2 passphrases can be up to 63 ASCII characters. MAX_PASSWORD_LEN must
 * include space for the null terminator, so the minimum valid value is 64.
 */
void test_password_len_satisfies_wpa2_max(void)
{
  /* 63 bytes payload + 1 null terminator */
  TEST_ASSERT_GREATER_OR_EQUAL(64, MAX_PASSWORD_LEN);
}

/**
 * @brief MAX_DEVICE_NAME_LEN must be large enough to hold the default name.
 */
void test_device_name_len_fits_default_name(void)
{
  /* +1 for null terminator */
  TEST_ASSERT_GREATER_OR_EQUAL(strlen(DEVICE_NAME) + 1, (size_t)MAX_DEVICE_NAME_LEN);
}

/**
 * @brief MAX_DEVICE_ID_LEN must allow a non-empty identifier.
 */
void test_device_id_len_is_sufficient(void)
{
  TEST_ASSERT_GREATER_THAN(0, MAX_DEVICE_ID_LEN);
}

/* =========================================================================
 * NVS key tests
 * ========================================================================= */

/**
 * @brief WIFI_SSID_KEY must be a non-empty string within NVS key limits.
 */
void test_nvs_wifi_ssid_key_is_valid(void)
{
  size_t len = strlen(WIFI_SSID_KEY);
  TEST_ASSERT_GREATER_THAN(0, len);
  /* NVS keys are limited to 15 characters + null terminator */
  TEST_ASSERT_LESS_THAN(NVS_KEY_MAX_LEN, len);
}

/**
 * @brief WIFI_PASS_KEY must be a non-empty string within NVS key limits.
 */
void test_nvs_wifi_pass_key_is_valid(void)
{
  size_t len = strlen(WIFI_PASS_KEY);
  TEST_ASSERT_GREATER_THAN(0, len);
  TEST_ASSERT_LESS_THAN(NVS_KEY_MAX_LEN, len);
}

/**
 * @brief DEVICE_NAME_KEY must be a non-empty string within NVS key limits.
 */
void test_nvs_device_name_key_is_valid(void)
{
  size_t len = strlen(DEVICE_NAME_KEY);
  TEST_ASSERT_GREATER_THAN(0, len);
  TEST_ASSERT_LESS_THAN(NVS_KEY_MAX_LEN, len);
}

/**
 * @brief DEVICE_ID_KEY must be a non-empty string within NVS key limits.
 */
void test_nvs_device_id_key_is_valid(void)
{
  size_t len = strlen(DEVICE_ID_KEY);
  TEST_ASSERT_GREATER_THAN(0, len);
  TEST_ASSERT_LESS_THAN(NVS_KEY_MAX_LEN, len);
}

/* =========================================================================
 * Default value tests
 * ========================================================================= */

/**
 * @brief DEVICE_NAME must be a non-empty string.
 */
void test_default_device_name_is_not_empty(void)
{
  TEST_ASSERT_GREATER_THAN(0, strlen(DEVICE_NAME));
}

/**
 * @brief DEVICE_NUMBER must be a positive value.
 */
void test_device_number_is_positive(void)
{
  TEST_ASSERT_GREATER_THAN(0, DEVICE_NUMBER);
}
