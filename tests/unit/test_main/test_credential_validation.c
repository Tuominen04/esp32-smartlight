/**
 * @file test_credential_validation.c
 * @brief Unit tests for WiFi credential validation rules
 *
 * Verifies the input validation rules that wifi_manager applies when
 * accepting credentials via BLE provisioning. Tests cover the happy path
 * (valid inputs) and boundary conditions (max-length values, empty strings,
 * NULL pointers) using only pure C logic — no hardware APIs required.
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
#include <stddef.h>
#include "unity.h"
#include "common_defs.h"

/* Size of the internal credentials JSON buffer in wifi_manager.c. */
#define WIFI_CREDENTIALS_BUFFER_SIZE 256

/* Overhead added by the JSON envelope: {"ssid":"","password":""} = 27 chars. */
#define CREDENTIAL_JSON_OVERHEAD 27

/* =========================================================================
 * Helpers — replicate the validation conditions from wifi_manager.c
 * so the tests remain independent of the implementation file.
 * ========================================================================= */

/** Returns true when a credential string passes the non-null, non-empty check. */
static bool credential_is_valid(const char *credential)
{
  return credential != NULL && strlen(credential) > 0;
}

/** Returns true when both SSID and password pass the combined validity check. */
static bool credentials_are_valid(const char *ssid, const char *password)
{
  return credential_is_valid(ssid) && credential_is_valid(password);
}

/** Returns true when a credential fits within the given buffer (including null). */
static bool credential_fits_buffer(const char *credential, size_t buf_size)
{
  return strnlen(credential, buf_size) < buf_size;
}

/* =========================================================================
 * Happy path tests
 * ========================================================================= */

/**
 * @brief A normal SSID and password should pass validation.
 */
void test_valid_credentials_accepted(void)
{
  TEST_ASSERT_TRUE(credentials_are_valid("MyNetwork", "MyPassword1"));
}

/**
 * @brief A single-character SSID should be accepted as non-empty.
 */
void test_single_char_ssid_accepted(void)
{
  TEST_ASSERT_TRUE(credential_is_valid("A"));
}

/**
 * @brief An SSID exactly at the usable limit (MAX_SSID_BUF_LEN - 1 chars) must
 *        still fit inside the MAX_SSID_BUF_LEN buffer with room for the null.
 */
void test_ssid_at_max_length_fits_buffer(void)
{
  char ssid[MAX_SSID_BUF_LEN];
  memset(ssid, 'A', MAX_SSID_BUF_LEN - 1);
  ssid[MAX_SSID_BUF_LEN - 1] = '\0';

  TEST_ASSERT_TRUE(credential_fits_buffer(ssid, MAX_SSID_BUF_LEN));
}

/**
 * @brief A password exactly at the usable limit (MAX_PASSWORD_LEN - 1 chars)
 *        must still fit inside the MAX_PASSWORD_LEN buffer.
 */
void test_password_at_max_length_fits_buffer(void)
{
  char password[MAX_PASSWORD_LEN];
  memset(password, 'B', MAX_PASSWORD_LEN - 1);
  password[MAX_PASSWORD_LEN - 1] = '\0';

  TEST_ASSERT_TRUE(credential_fits_buffer(password, MAX_PASSWORD_LEN));
}

/**
 * @brief A max-length SSID and max-length password, combined with the JSON
 *        envelope, must fit in the 256-byte credentials buffer used by
 *        wifi_manager when receiving credentials over BLE.
 */
void test_max_credentials_fit_json_buffer(void)
{
  /* Usable payload lengths (without null terminators). */
  size_t max_ssid_payload     = (size_t)(MAX_SSID_BUF_LEN - 1);
  size_t max_password_payload = (size_t)(MAX_PASSWORD_LEN - 1);
  size_t total                = max_ssid_payload + max_password_payload + CREDENTIAL_JSON_OVERHEAD;

  TEST_ASSERT_LESS_OR_EQUAL(WIFI_CREDENTIALS_BUFFER_SIZE, total);
}

/* =========================================================================
 * Rejection tests
 * ========================================================================= */

/**
 * @brief An empty SSID string must be rejected.
 */
void test_empty_ssid_rejected(void)
{
  TEST_ASSERT_FALSE(credential_is_valid(""));
}

/**
 * @brief An empty password string must be rejected.
 */
void test_empty_password_rejected(void)
{
  TEST_ASSERT_FALSE(credentials_are_valid("ValidSSID", ""));
}

/**
 * @brief A NULL SSID pointer must be rejected without crashing.
 */
void test_null_ssid_rejected(void)
{
  TEST_ASSERT_FALSE(credential_is_valid(NULL));
}

/**
 * @brief A NULL password with a valid SSID must be rejected.
 */
void test_null_password_rejected(void)
{
  TEST_ASSERT_FALSE(credentials_are_valid("ValidSSID", NULL));
}

/* =========================================================================
 * Boundary tests
 * ========================================================================= */

/**
 * @brief An SSID one character longer than the usable limit must not fit in
 *        the MAX_SSID_BUF_LEN buffer (overflow detection).
 */
void test_ssid_over_max_length_does_not_fit(void)
{
  /* MAX_SSID_BUF_LEN bytes of 'A' — no room for null terminator. */
  char ssid[MAX_SSID_BUF_LEN + 1];
  memset(ssid, 'A', MAX_SSID_BUF_LEN);
  ssid[MAX_SSID_BUF_LEN] = '\0';

  TEST_ASSERT_FALSE(credential_fits_buffer(ssid, MAX_SSID_BUF_LEN));
}

/**
 * @brief A password one character longer than the usable limit must not fit.
 */
void test_password_over_max_length_does_not_fit(void)
{
  char password[MAX_PASSWORD_LEN + 1];
  memset(password, 'B', MAX_PASSWORD_LEN);
  password[MAX_PASSWORD_LEN] = '\0';

  TEST_ASSERT_FALSE(credential_fits_buffer(password, MAX_PASSWORD_LEN));
}
