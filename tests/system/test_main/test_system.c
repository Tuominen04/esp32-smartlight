/**
 * @file test_system.c
 * @brief ESP-IDF Unity-based system tests with fixed GPIO test
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
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_chip_info.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DEFAULT_GPIO GPIO_NUM_8 // Use GPIO 8 for testing

static const char* TEST_TAG = "SYSTEM_TEST";
static const char* TEST_NAME_TAG = "SYSTEM_TEST_NAME";

/**
 * @brief Test that basic system is working
 */
void test_basic_system_functionality(void)
{
    ESP_LOGI(TEST_NAME_TAG, "Testing basic system functionality");
    
    // Test 1: Check ESP chip info  
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    TEST_ASSERT_TRUE(chip_info.cores >= 1);
    ESP_LOGI(TEST_TAG, "✅ Chip has %d cores", chip_info.cores);
    
    // Test 2: Check free heap
    size_t free_heap = esp_get_free_heap_size();
    TEST_ASSERT_TRUE(free_heap > 10000);
    ESP_LOGI(TEST_TAG, "✅ Free heap: %zu bytes", free_heap);
    
    // Test 3: Basic string operations
    char test_string[32];
    sprintf(test_string, "Test_%d", 123);
    TEST_ASSERT_EQUAL_STRING("Test_123", test_string);
    ESP_LOGI(TEST_TAG, "✅ String operations work");
}

/**
 * @brief Test NVS basic functionality
 */
void test_nvs_basic_functionality(void)
{
    ESP_LOGI(TEST_NAME_TAG, "Testing NVS basic functionality");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    ESP_LOGI(TEST_TAG, "✅ NVS initialized successfully");
    
    // Test basic NVS operations
    nvs_handle_t nvs_handle;
    ret = nvs_open("test", NVS_READWRITE, &nvs_handle);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    
    // Write a test value
    int32_t test_value = 42;
    ret = nvs_set_i32(nvs_handle, "test_key", test_value);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    
    ret = nvs_commit(nvs_handle);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    
    // Read it back
    int32_t read_value = 0;
    ret = nvs_get_i32(nvs_handle, "test_key", &read_value);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL(test_value, read_value);
    
    nvs_close(nvs_handle);
    ESP_LOGI(TEST_TAG, "✅ NVS read/write operations work");
}

/**
 * @brief Test GPIO basic functionality with better debugging
 */
void test_gpio_basic_functionality(void)
{
    ESP_LOGI(TEST_NAME_TAG, "Testing GPIO basic functionality");
    
    // First, reset the GPIO to ensure clean state
    gpio_reset_pin(DEFAULT_GPIO);
    
    // Test GPIO configuration
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pin_bit_mask = (1ULL << DEFAULT_GPIO), // Use GPIO 8 
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    TEST_ASSERT_EQUAL_MESSAGE(ESP_OK, ret, "GPIO config failed");
    ESP_LOGI(TEST_TAG, "✅ GPIO configured successfully");
    
    // Test GPIO set/get operations with debug output
    ESP_LOGI(TEST_TAG, "Setting GPIO %d to HIGH (1)", DEFAULT_GPIO);
    ret = gpio_set_level(DEFAULT_GPIO, 1);
    TEST_ASSERT_EQUAL_MESSAGE(ESP_OK, ret, "Failed to set GPIO level to 1");
    
    // Small delay to ensure GPIO settles
    vTaskDelay(pdMS_TO_TICKS(10));
    
    int level = gpio_get_level(DEFAULT_GPIO);
    ESP_LOGI(TEST_TAG, "Read GPIO %d level: %d (expected: 1)", DEFAULT_GPIO, level);
    TEST_ASSERT_EQUAL_MESSAGE(1, level, "GPIO read value doesn't match expected HIGH");
    
    ESP_LOGI(TEST_TAG, "Setting GPIO %d to LOW (0)", DEFAULT_GPIO);
    ret = gpio_set_level(DEFAULT_GPIO, 0);
    TEST_ASSERT_EQUAL_MESSAGE(ESP_OK, ret, "Failed to set GPIO level to 0");
    
    // Small delay to ensure GPIO settles
    vTaskDelay(pdMS_TO_TICKS(10));
    
    level = gpio_get_level(DEFAULT_GPIO);
    ESP_LOGI(TEST_TAG, "Read GPIO %d level: %d (expected: 0)", DEFAULT_GPIO, level);
    TEST_ASSERT_EQUAL_MESSAGE(0, level, "GPIO read value doesn't match expected LOW");
    
    ESP_LOGI(TEST_TAG, "✅ GPIO set/get operations work");
}

/**
 * @brief Test memory allocation
 */
void test_memory_allocation(void)
{
    ESP_LOGI(TEST_NAME_TAG, "Testing memory allocation");
    
    // Test malloc/free
    void* test_ptr = malloc(1024);
    TEST_ASSERT_NOT_NULL(test_ptr);
    
    // Write some data to make sure it's real memory
    memset(test_ptr, 0xAA, 1024);
    uint8_t* byte_ptr = (uint8_t*)test_ptr;
    TEST_ASSERT_EQUAL(0xAA, byte_ptr[0]);
    TEST_ASSERT_EQUAL(0xAA, byte_ptr[1023]);
    
    free(test_ptr);
    ESP_LOGI(TEST_TAG, "✅ Memory allocation works");
}
