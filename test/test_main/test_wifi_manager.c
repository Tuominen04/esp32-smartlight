/**
 * @file test_wifi_manager.c
 * @brief Unit tests for WiFi Manager module
 * 
 * Note: These tests focus on testing the logic and state management
 * of the WiFi manager without requiring actual WiFi hardware.
 */

#include <string.h>
#include "unity.h"
#include "esp_log.h"
#include "esp_err.h"
#include "cJSON.h"
#include "wifi/wifi_manager.h"
#include "storage/nvs_manager.h"
#include "common/common_defs.h"

static const char* TEST_TAG = "TEST_WIFI";

// Test credentials
static const char* TEST_SSID = "TestNetwork";
static const char* TEST_PASSWORD = "TestPassword123";
static const char* TEST_JSON_CREDS = "{\"ssid\":\"TestNetwork\",\"password\":\"TestPassword123\"}";
static const char* INVALID_JSON = "invalid json data";

static bool connect_called = false;
static bool disconnect_called = false;

// Define test callbacks
static void test_wifi_connect_cb(void) {
    connect_called = true;
}

static void test_wifi_disconnect_cb(void) {
    disconnect_called = true;
}

/**
 * @brief Test WiFi manager initialization
 */
void test_wifi_manager_init(void)
{
    ESP_LOGI(TEST_TAG, "Testing WiFi manager initialization");
    
    esp_err_t result = wifi_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, result);
    
    // Test double initialization (should not fail)
    result = wifi_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, result);
}

/**
 * @brief Test initial connection state
 */
void test_wifi_initial_connection_state(void)
{
    ESP_LOGI(TEST_TAG, "Testing initial connection state");
    
    esp_err_t res = wifi_manager_init();
    if(res != ESP_OK) {
        TEST_FAIL_MESSAGE("WiFi manager initialization failed");
    }
    
    // Initially should not be connected
    bool connected = wifi_manager_is_connected();
    TEST_ASSERT_FALSE(connected);
}

/**
 * @brief Test setting new credentials
 */
void test_wifi_set_new_credentials(void)
{
    ESP_LOGI(TEST_TAG, "Testing setting new credentials");
    
    esp_err_t res = wifi_manager_init();
    if(res != ESP_OK) {
        TEST_FAIL_MESSAGE("WiFi manager initialization failed");
    }
    
    // Initially should have no new credentials
    bool has_new = wifi_manager_has_new_credentials();
    TEST_ASSERT_FALSE(has_new);
    
    // Set new credentials
    wifi_manager_set_new_credentials(TEST_JSON_CREDS);
    
    // Should now have new credentials
    has_new = wifi_manager_has_new_credentials();
    TEST_ASSERT_TRUE(has_new);
}

/**
 * @brief Test setting invalid credentials
 */
void test_wifi_set_invalid_credentials(void)
{
    ESP_LOGI(TEST_TAG, "Testing setting invalid credentials");
    
    esp_err_t res = wifi_manager_init();
    if(res != ESP_OK) {
        TEST_FAIL_MESSAGE("WiFi manager initialization failed");
    }
    
    // Test NULL credentials
    wifi_manager_set_new_credentials(NULL);
    bool has_new = wifi_manager_has_new_credentials();
    TEST_ASSERT_FALSE(has_new);
    
    // Test empty string
    wifi_manager_set_new_credentials("");
    has_new = wifi_manager_has_new_credentials();
    TEST_ASSERT_FALSE(has_new);
}

/**
 * @brief Test credential string length limits
 */
void test_wifi_credential_length_limits(void)
{
    ESP_LOGI(TEST_TAG, "Testing credential length limits");
    
    esp_err_t res = wifi_manager_init();
    if(res != ESP_OK) {
        TEST_FAIL_MESSAGE("WiFi manager initialization failed");
    }
    
    // Create a very long credential string (longer than buffer)
    char long_creds[300];
    memset(long_creds, 'A', sizeof(long_creds) - 1);
    long_creds[sizeof(long_creds) - 1] = '\0';
    
    // Should handle gracefully
    wifi_manager_set_new_credentials(long_creds);
    bool has_new = wifi_manager_has_new_credentials();
    TEST_ASSERT_FALSE(has_new); // Should reject overly long credentials
}

/**
 * @brief Test JSON parsing validation
 */
void test_wifi_json_credential_format(void)
{
    ESP_LOGI(TEST_TAG, "Testing JSON credential format validation");
    
    // Test valid JSON parsing
    cJSON *root = cJSON_Parse(TEST_JSON_CREDS);
    TEST_ASSERT_NOT_NULL(root);
    
    cJSON *ssid_item = cJSON_GetObjectItem(root, "ssid");
    cJSON *pass_item = cJSON_GetObjectItem(root, "password");
    
    TEST_ASSERT_NOT_NULL(ssid_item);
    TEST_ASSERT_NOT_NULL(pass_item);
    TEST_ASSERT_EQUAL_STRING(TEST_SSID, ssid_item->valuestring);
    TEST_ASSERT_EQUAL_STRING(TEST_PASSWORD, pass_item->valuestring);
    
    cJSON_Delete(root);
    
    // Test invalid JSON
    cJSON *invalid_root = cJSON_Parse(INVALID_JSON);
    TEST_ASSERT_NULL(invalid_root);
}

/**
 * @brief Test saved credentials functionality
 */
void test_wifi_saved_credentials(void)
{
    ESP_LOGI(TEST_TAG, "Testing saved credentials functionality");
    
    esp_err_t res = wifi_manager_init();
    if(res != ESP_OK) {
        TEST_FAIL_MESSAGE("WiFi manager initialization failed");
    }
    
    // First save some credentials via NVS manager
    esp_err_t result = nvs_manager_save_wifi_credentials(TEST_SSID, TEST_PASSWORD);
    TEST_ASSERT_EQUAL(ESP_OK, result);
    
    // Test retrieving saved credentials
    char retrieved_ssid[MAX_SSID_LEN] = {0};
    char retrieved_password[MAX_PASSWORD_LEN] = {0};
    
    result = wifi_manager_get_saved_credentials(
        retrieved_ssid, sizeof(retrieved_ssid),
        retrieved_password, sizeof(retrieved_password)
    );
    
    TEST_ASSERT_EQUAL(ESP_OK, result);
    TEST_ASSERT_EQUAL_STRING(TEST_SSID, retrieved_ssid);
    TEST_ASSERT_EQUAL_STRING(TEST_PASSWORD, retrieved_password);
}

/**
 * @brief Test no saved credentials scenario
 */
void test_wifi_no_saved_credentials(void)
{
    ESP_LOGI(TEST_TAG, "Testing no saved credentials scenario");
    
    esp_err_t res = wifi_manager_init();
    if(res != ESP_OK) {
        TEST_FAIL_MESSAGE("WiFi manager initialization failed");
    }
    
    // Clear any existing credentials
    nvs_manager_delete_wifi_credentials();
    
    // Try to get saved credentials
    char retrieved_ssid[MAX_SSID_LEN] = {0};
    char retrieved_password[MAX_PASSWORD_LEN] = {0};
    
    esp_err_t result = wifi_manager_get_saved_credentials(
        retrieved_ssid, sizeof(retrieved_ssid),
        retrieved_password, sizeof(retrieved_password)
    );
    
    TEST_ASSERT_EQUAL(ESP_FAIL, result);
}

/**
 * @brief Test disconnect functionality
 */
void test_wifi_disconnect(void)
{
    ESP_LOGI(TEST_TAG, "Testing disconnect functionality");
    
    esp_err_t res = wifi_manager_init();
    printf("\nDEBUG init: %s\n", esp_err_to_name(res));

    TEST_ASSERT_EQUAL_MESSAGE(ESP_OK, res, "WiFi manager initialization failed");
    
    // Test disconnect (should not fail even if not connected)
    res = wifi_manager_disconnect();
    printf("\nDEBUG disconnect: %s\n", esp_err_to_name(res));
    TEST_ASSERT_EQUAL_MESSAGE(ESP_OK, res, "WiFi disconnect failed - check if WiFi stack is properly initialized");
        
    // Additional check
    bool is_connected = wifi_manager_is_connected();
    printf("\nDEBUG is connect: %d\n", is_connected);
    TEST_ASSERT_FALSE_MESSAGE(is_connected, "WiFi should not be connected after disconnect");
}

/**
 * @brief Test callback setting
 */
void test_wifi_callback_setting(void)
{
    ESP_LOGI(TEST_TAG, "Testing callback setting");

    esp_err_t res = wifi_manager_init();
    if(res != ESP_OK) {
        TEST_FAIL_MESSAGE("WiFi manager initialization failed");
    }

    // Set callbacks (should not crash)
    wifi_manager_set_callbacks(test_wifi_connect_cb, test_wifi_disconnect_cb);

    // Test setting NULL callbacks (should not crash)
    wifi_manager_set_callbacks(NULL, NULL);

    TEST_ASSERT_TRUE(true); // If we get here, callbacks were set successfully
}

/**
 * @brief Test event group access
 */
void test_wifi_event_group_access(void)
{
    ESP_LOGI(TEST_TAG, "Testing event group access");
    
    esp_err_t res = wifi_manager_init();
    if(res != ESP_OK) {
        TEST_FAIL_MESSAGE("WiFi manager initialization failed");
    }
    
    // Get event group handle
    EventGroupHandle_t event_group = wifi_manager_get_event_group();
    TEST_ASSERT_NOT_NULL(event_group);
}
