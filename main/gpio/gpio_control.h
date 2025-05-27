#ifndef GPIO_CONTROL_H
#define GPIO_CONTROL_H

#include "driver/gpio.h"
#include <stdbool.h>

// GPIO pin definitions - move these from main file
#define LED_GPIO 23  // Adjust based on your board

// Public functions
esp_err_t gpio_control_init(void);
void gpio_set_light_state(bool state);
bool gpio_get_light_state(void);
void gpio_toggle_light(void);

#endif // GPIO_CONTROL_H