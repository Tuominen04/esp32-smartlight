idf_component_register(
    SRCS
        "test_main.c"
        "test_nvs_manager.c"
        "test_wifi_manager.c"
        "test_gpio_control.c"
        "test_device_info.c"

    INCLUDE_DIRS "."

    REQUIRES
        unity
        gpio
        storage
        system_info
        wifi
)
    