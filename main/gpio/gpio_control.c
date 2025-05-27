#include "gpio_control.h"
#include "esp_log.h"


static const char *GPIO_TAG = "GPIO_CONTROL";
static bool light_state = false;

esp_err_t gpio_control_init(void)
{
    ESP_LOGI(GPIO_TAG, "Initializing GPIO control");
    
    // Initialize GPIO
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, light_state);
    
    ESP_LOGI(GPIO_TAG, "GPIO control initialized, LED pin: %d", LED_GPIO);
    return ESP_OK;
}

void gpio_set_light_state(bool state)
{
    light_state = state;
    gpio_set_level(LED_GPIO, light_state);
    ESP_LOGI(GPIO_TAG, "Light set to %s", light_state ? "ON" : "OFF");
}

bool gpio_get_light_state(void)
{
    return light_state;
}

void gpio_toggle_light(void)
{
    light_state = !light_state;
    gpio_set_level(LED_GPIO, light_state);
    ESP_LOGI(GPIO_TAG, "Light toggled to %s", light_state ? "ON" : "OFF");
}