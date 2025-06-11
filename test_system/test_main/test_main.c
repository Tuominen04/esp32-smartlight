/**
 * @file test_main.c
 * @brief Unity test runner for ESP-IDF - Manual approach
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
#include "unity_internals.h"
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
 * @brief Test a simple assertion failure with clear message
 */
void test_simple_failure(void)
{
    ESP_LOGI(TAG, "Running simple failure test");
    TEST_ASSERT_TRUE_MESSAGE(false, "This boolean test should fail");
}

/**
 * @brief Test a simple assertion success
 */
void test_simple_success(void)
{
    ESP_LOGI(TAG, "Running simple success test");
    TEST_ASSERT_TRUE_MESSAGE(true, "This boolean test should pass");
}

/**
 * @brief Custom test runner that captures Unity output properly
 */
void run_test_with_output(UnityTestFunction test, const char* name, const UNITY_LINE_TYPE line)
{
    printf("\n🧪 TESTING: %s\n", name);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    // Store current test state
    UNITY_UINT testsPassed = Unity.NumberOfTests - Unity.TestFailures;
    
    // Run the test - Unity will handle the assertions
    UnityDefaultTestRun(test, name, line);
    
    // Check if test passed or failed
    UNITY_UINT newTestsPassed = Unity.NumberOfTests - Unity.TestFailures;
    if (newTestsPassed > testsPassed) {
        printf("🟢 RESULT: PASSED\n");
    } else {
        printf("🔴 RESULT: FAILED\n");
    }
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    fflush(stdout);
}

// Redefine RUN_TEST to use our custom runner
#define RUN_TEST(func) run_test_with_output(func, #func, __LINE__)

/**
 * @brief Print detailed test results including failure information
 */
void print_detailed_test_summary(void)
{
    printf("\n");
    printf("===============================================\n");
    printf("               TEST RESULTS\n");
    printf("===============================================\n");
    printf("Summary: %d total, %d passed, %d failed\n", 
           Unity.NumberOfTests, 
           Unity.NumberOfTests - Unity.TestFailures,
           Unity.TestFailures);
    printf("===============================================\n");
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
    RUN_TEST(test_deliberate_failure);
    
    // ===== System Tests =====
    ESP_LOGI(TAG, "\n=== Running System Tests ===");
    RUN_TEST(test_basic_system_functionality);
    RUN_TEST(test_nvs_basic_functionality);
    RUN_TEST(test_gpio_basic_functionality);
    RUN_TEST(test_memory_allocation);
    
    // Print detailed results
    print_detailed_test_summary();
    
    UNITY_END();
    
    ESP_LOGI(TAG, "=================================================");
    ESP_LOGI(TAG, "🎉 All system tests completed!");
    ESP_LOGI(TAG, "=================================================");
    
    // Keep the program running so we can see all output
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
