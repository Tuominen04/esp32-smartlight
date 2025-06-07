#include "gpio_control.h"
#include "esp_log.h"

static const char *GPIO_TAG = "GPIO_CONTROL";

/** Current state of the LED light (true = ON, false = OFF). */
static bool light_state = false;

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
    ESP_LOGI(GPIO_TAG, "Initializing GPIO control");
    
    // Initialize GPIO
    esp_err_t res = gpio_reset_pin(LED_GPIO);
    if (res != ESP_OK) {
        ESP_LOGE(GPIO_TAG, "Failed to reset GPIO pin %d: %s", LED_GPIO, esp_err_to_name(res));
        return res;
    }

    // Configure GPIO mode
    res = gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    if (res != ESP_OK) {
        ESP_LOGE(GPIO_TAG, "Failed to set GPIO direction for pin %d: %s", LED_GPIO, esp_err_to_name(res));
        return res;
    }

    // Set the initial state of the light to OFF
    res = gpio_set_level(LED_GPIO, light_state);
    if (res != ESP_OK) {
        ESP_LOGE(GPIO_TAG, "Failed to set GPIO level for pin %d: %s", LED_GPIO, esp_err_to_name(res));
        return res;
    }
    
    ESP_LOGI(GPIO_TAG, "GPIO control initialized, LED pin: %d", LED_GPIO);
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
    light_state = state;
    esp_err_t res = gpio_set_level(LED_GPIO, light_state);
    if (res != ESP_OK) {
        ESP_LOGE(GPIO_TAG, "Failed to set GPIO level for pin %d: %s", LED_GPIO, esp_err_to_name(res));
        return;
    }

    ESP_LOGI(GPIO_TAG, "Light set to %s", light_state ? "ON" : "OFF");
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
    return light_state;
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
    light_state = !light_state;
    esp_err_t res = gpio_set_level(LED_GPIO, light_state);
    if (res != ESP_OK) {
        ESP_LOGE(GPIO_TAG, "Failed to toggle GPIO level for pin %d: %s", LED_GPIO, esp_err_to_name(res));
        return;
    }
    ESP_LOGI(GPIO_TAG, "Light toggled to %s", light_state ? "ON" : "OFF");
}
