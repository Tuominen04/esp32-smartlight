idf_component_register(
  SRCS "gpio_control.c"
  INCLUDE_DIRS "."
  REQUIRES esp_driver_gpio log
)
