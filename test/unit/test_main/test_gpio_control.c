/**
 * @file test_gpio_control.c
 * @brief Unit tests for GPIO Control module
 * 
 * Copyright (c) 2025 Arttu Tuominen. All rights reserved.
 * 
 * This software is licensed under commercial terms.
 * See LICENSE file for complete license terms.
 * 
 * NOTICE: This file contains proprietary information. Unauthorized 
 * distribution or use is strictly prohibited.
 */

#include "unity.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "gpio_control.h"

static const char* TEST_TAG = "TEST_GPIO";

/**
 * @brief Test GPIO initialization
 */
void test_gpio_init(void)
{
    ESP_LOGI(TEST_TAG, "Testing GPIO initialization");
    
    esp_err_t res = gpio_control_init();
    TEST_ASSERT_EQUAL(ESP_OK, res);
}

/**
 * @brief Test light state control
 */
void test_gpio_light_state_control(void)
{
    ESP_LOGI(TEST_TAG, "Testing light state control");
    
    // Initialize GPIO first
    esp_err_t res = gpio_control_init();
    if(res != ESP_OK) {
        TEST_FAIL_MESSAGE("GPIO initialization failed");
    }

    // Test setting light ON
    gpio_set_light_state(true);
    bool state = gpio_get_light_state();
    TEST_ASSERT_TRUE(state);
    
    // Test setting light OFF
    gpio_set_light_state(false);
    state = gpio_get_light_state();
    TEST_ASSERT_FALSE(state);
}

/**
 * @brief Test light toggle functionality
 */
void test_gpio_light_toggle(void)
{
    ESP_LOGI(TEST_TAG, "Testing light toggle functionality");
    
    // Initialize GPIO first
    esp_err_t res = gpio_control_init();
    if(res != ESP_OK) {
        TEST_FAIL_MESSAGE("GPIO initialization failed");
    }
    
    // Start with known state (OFF)
    gpio_set_light_state(false);
    bool initial_state = gpio_get_light_state();
    TEST_ASSERT_FALSE(initial_state);
    
    // Toggle and verify state changed
    gpio_toggle_light();
    bool toggled_state = gpio_get_light_state();
    TEST_ASSERT_TRUE(toggled_state);
    TEST_ASSERT_NOT_EQUAL(initial_state, toggled_state);
    
    // Toggle again and verify it's back to original
    gpio_toggle_light();
    bool final_state = gpio_get_light_state();
    TEST_ASSERT_FALSE(final_state);
    TEST_ASSERT_EQUAL(initial_state, final_state);
}

/**
 * @brief Test multiple rapid state changes
 */
void test_gpio_rapid_state_changes(void)
{
    ESP_LOGI(TEST_TAG, "Testing rapid state changes");
    
    esp_err_t res = gpio_control_init();
    if(res != ESP_OK) {
        TEST_FAIL_MESSAGE("GPIO initialization failed");
    }
    
    // Perform rapid state changes
    for (int i = 0; i < 100; i++) {
        bool expected_state = (i % 2 == 0);
        gpio_set_light_state(expected_state);
        
        bool actual_state = gpio_get_light_state();
        TEST_ASSERT_EQUAL(expected_state, actual_state);
    }
}

/**
 * @brief Test state consistency after multiple operations
 */
void test_gpio_state_consistency(void)
{
    ESP_LOGI(TEST_TAG, "Testing state consistency");
    
    esp_err_t res = gpio_control_init();
    if(res != ESP_OK) {
        TEST_FAIL_MESSAGE("GPIO initialization failed");
    }
    
    // Test sequence: OFF -> ON -> TOGGLE -> TOGGLE -> OFF
    gpio_set_light_state(false);
    TEST_ASSERT_FALSE(gpio_get_light_state());
    
    gpio_set_light_state(true);
    TEST_ASSERT_TRUE(gpio_get_light_state());
    
    gpio_toggle_light(); // Should be OFF now
    TEST_ASSERT_FALSE(gpio_get_light_state());
    
    gpio_toggle_light(); // Should be ON now
    TEST_ASSERT_TRUE(gpio_get_light_state());
    
    gpio_set_light_state(false); // Back to OFF
    TEST_ASSERT_FALSE(gpio_get_light_state());
}

/**
 * @brief Test initial state after initialization
 */
void test_gpio_initial_state(void)
{
    ESP_LOGI(TEST_TAG, "Testing initial state after initialization");
    
    // Re-initialize GPIO
    esp_err_t result = gpio_control_init();
    TEST_ASSERT_EQUAL(ESP_OK, result);
    
    // Initial state should be OFF (false)
    bool initial_state = gpio_get_light_state();
    TEST_ASSERT_FALSE(initial_state);
    
    // GPIO level should also be 0
    int gpio_level = gpio_get_level(LED_GPIO);
    TEST_ASSERT_EQUAL(0, gpio_level);
}

/**
 * @brief Performance test - measure toggle speed
 */
void test_gpio_toggle_performance(void)
{
    ESP_LOGI(TEST_TAG, "Testing toggle performance");
    
    esp_err_t res = gpio_control_init();
    if(res != ESP_OK) {
        TEST_FAIL_MESSAGE("GPIO initialization failed");
    }
    
    const int NUM_TOGGLES = 1000;
    int64_t start_time = esp_timer_get_time();
    
    for (int i = 0; i < NUM_TOGGLES; i++) {
        gpio_toggle_light();
    }
    
    int64_t end_time = esp_timer_get_time();
    int64_t duration_us = end_time - start_time;
    
    ESP_LOGI(TEST_TAG, "Performed %d toggles in %lld microseconds", 
             NUM_TOGGLES, duration_us);
    ESP_LOGI(TEST_TAG, "Average time per toggle: %.2f microseconds", 
             (float)duration_us / NUM_TOGGLES);
    
    // Sanity check - should be less than 5s per 1000 toggles (allowing for logging overhead)
    TEST_ASSERT_LESS_THAN(5000000, (int)duration_us); // 5 seconds total max
}
