/**
 * @file test_main.c
 * @brief Unity test runner for ESP-IDF system tests
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
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "TEST_RUNNER";

// Forward declarations of test functions
void test_basic_system_functionality(void);
void test_nvs_basic_functionality(void);
void test_gpio_basic_functionality(void);
void test_memory_allocation(void);
void test_deliberate_failure(void);
void test_deliberate_success(void);

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
 * @brief Test that deliberately fails to verify Unity is working
 */
void test_deliberate_failure(void)
{
    ESP_LOGI(TAG, "Testing deliberate failure to verify Unity");
    ESP_LOGE(TAG, "⚠️  This test is SUPPOSED to fail!");

    // This should definitely fail
    TEST_ASSERT_EQUAL_MESSAGE(1, 2, "Deliberate failure test - 1 should not equal 2");

    ESP_LOGE(TAG, "❌ If you see this message, Unity didn't catch the failure!");
}

/**
 * @brief Test that should pass to verify Unity can detect success
 */
void test_deliberate_success(void)
{
    ESP_LOGI(TAG, "Testing deliberate success to verify Unity");
    ESP_LOGI(TAG, "✅ This test should pass!");

    TEST_ASSERT_EQUAL(2, 2);  // 2 equals 2

    ESP_LOGI(TAG, "✅ Success test completed!");
}

/**
 * @brief Main application entry point for tests
 */
void app_main(void)
{
    ESP_LOGI(TAG, "🚀 Starting ESP32 Light Controller System Tests");
    ESP_LOGI(TAG, "=================================================");

    // Small delay to let system stabilize
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Initialize Unity test framework
    UNITY_BEGIN();

    // ===== Unity Verification Tests =====
    ESP_LOGI(TAG, "\n=== Running Unity Verification Tests ===");
    RUN_TEST(test_deliberate_success);
#ifdef CONFIG_TEST_DELIBERATE_FAIL
    RUN_TEST(test_deliberate_failure);
#endif

    // ===== System Tests =====
    ESP_LOGI(TAG, "\n=== Running System Tests ===");
    RUN_TEST(test_basic_system_functionality);
    RUN_TEST(test_nvs_basic_functionality);
    RUN_TEST(test_gpio_basic_functionality);
    RUN_TEST(test_memory_allocation);

    UNITY_END();

    ESP_LOGI(TAG, "=================================================");
    ESP_LOGI(TAG, "🎉 All system tests completed!");
    ESP_LOGI(TAG, "=================================================");
}
