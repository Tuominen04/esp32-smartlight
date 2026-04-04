/**
 * @file gpio_control.c
 * @brief GPIO control implementation for LED light
 * 
 * Copyright (c) 2025 Arttu Tuominen. All rights reserved.
 * 
 * This software is licensed under commercial terms.
 * See LICENSE file for complete license terms.
 * 
 * NOTICE: This file contains proprietary information. Unauthorized 
 * distribution or use is strictly prohibited.
 */

#include "gpio_control.h"
#include "esp_log.h"

static const char *gpio_tag = "GPIO_CONTROL";

/** Software mirror of the GPIO output level. Avoids reading back the input register,
 *  which is unreliable for output-only pins on ESP32-C6. */
static bool s_light_state = false;

/**
 * @brief Set LED GPIO output level and log failures.
 *
 * @param[in] level  Target GPIO level (0 = OFF, 1 = ON).
 *
 * @return
 *      - ESP_OK on success
 *      - ESP_ERR_* returned by gpio_set_level on failure
 */
static esp_err_t gpio_set_led_level(uint32_t level)
{
  esp_err_t res = gpio_set_level(LED_GPIO, (uint32_t)(level ? 1U : 0U));
  if (res != ESP_OK) {
    ESP_LOGE(gpio_tag, "Failed to set GPIO level for pin %d: %s", LED_GPIO, esp_err_to_name(res));
  }
  return res;
}

/**
 * @brief Initialize GPIO control for LED light management.
 *
 * This function configures the LED GPIO pin for output mode and sets it to the
 * current light state. It initializes the hardware necessary for controlling
 * the LED light through GPIO operations.
 *
 * @return
 *      - ESP_OK on successful GPIO initialization
 *      - ESP_ERR_* on failure during GPIO configuration
 *
 * @note The LED GPIO pin is defined by LED_GPIO macro in the header file.
 *       The initial light state is set to OFF (false).
 */
esp_err_t gpio_control_init(void)
{
  ESP_LOGI(gpio_tag, "Initializing GPIO control");
  
  // Initialize GPIO
  esp_err_t res = gpio_reset_pin(LED_GPIO);
  if (res != ESP_OK) {
    ESP_LOGE(gpio_tag, "Failed to reset GPIO pin %d: %s", LED_GPIO, esp_err_to_name(res));
    return res;
  }

  // Configure GPIO mode
  res = gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
  if (res != ESP_OK) {
    ESP_LOGE(gpio_tag, "Failed to set GPIO direction for pin %d: %s", LED_GPIO, esp_err_to_name(res));
    return res;
  }

  // Set the initial state of the light to OFF
  s_light_state = false;
  res = gpio_set_led_level(0U);
  if (res != ESP_OK) {
    return res;
  }
  
  ESP_LOGI(gpio_tag, "GPIO control initialized, LED pin: %d", LED_GPIO);
  return ESP_OK;
}

/**
 * @brief Set the light state to ON or OFF.
 *
 * Updates both the internal light state variable and the physical GPIO pin
 * to match the requested state. The function provides immediate control
 * over the LED light with logging for debugging purposes.
 *
 * @param[in] state  The desired light state (true = ON, false = OFF).
 *
 * @note This function directly controls the GPIO pin level and updates
 *       the internal state tracker for consistency.
 */
void gpio_set_light_state(bool state)
{
  esp_err_t res = gpio_set_led_level((uint32_t)(state ? 1U : 0U));
  if (res != ESP_OK) {
    return;
  }
  s_light_state = state;
  ESP_LOGI(gpio_tag, "Light set to %s", state ? "ON" : "OFF");
}

/**
 * @brief Get the current light state.
 *
 * Returns the current state of the LED light as stored in the internal
 * state variable. This reflects the last commanded state of the light.
 *
 * @return
 *      - true if the light is currently ON
 *      - false if the light is currently OFF
 *
 * @note This function returns the software state, which should match
 *       the actual hardware state if properly synchronized.
 */
bool gpio_get_light_state(void)
{
  return s_light_state;
}

/**
 * @brief Toggle the current light state.
 *
 * Switches the light from ON to OFF or from OFF to ON, updating both
 * the internal state variable and the physical GPIO pin. This provides
 * a convenient way to alternate the light state without needing to
 * track the current state externally.
 *
 * @note The function logs the new state after toggling for debugging
 *       and monitoring purposes.
 */
void gpio_toggle_light(void)
{
  bool next_state = !s_light_state;
  esp_err_t res = gpio_set_led_level((uint32_t)(next_state ? 1U : 0U));
  if (res != ESP_OK) {
    return;
  }
  s_light_state = next_state;
  ESP_LOGI(gpio_tag, "Light toggled to %s", next_state ? "ON" : "OFF");
}
