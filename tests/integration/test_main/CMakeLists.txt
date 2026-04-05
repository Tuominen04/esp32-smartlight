idf_component_register(
  SRCS
    "test_main.c"
    "test_device_nvs_integration.c"
    "test_wifi_nvs_integration.c"

  INCLUDE_DIRS "."

  REQUIRES
    unity
    storage
    system_info
    wifi
)
