/**
 * @file gpio_control.h
 * @brief GPIO control API for LED light
 * 
 * Copyright (c) 2025 Arttu Tuominen. All rights reserved.
 * 
 * This software is licensed under commercial terms.
 * See LICENSE file for complete license terms.
 * 
 * NOTICE: This file contains proprietary information. Unauthorized 
 * distribution or use is strictly prohibited.
 */

#ifndef COMPONENTS_GPIO_GPIO_CONTROL_H
#define COMPONENTS_GPIO_GPIO_CONTROL_H

#include "esp_err.h"
#include "driver/gpio.h"
#include <stdbool.h>

/** GPIO pin definition for LED output. */
#define LED_GPIO 23  // Adjust based on your board

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
esp_err_t gpio_control_init(void);

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
void gpio_set_light_state(bool state);

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
bool gpio_get_light_state(void);

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
void gpio_toggle_light(void);

#endif // COMPONENTS_GPIO_GPIO_CONTROL_H