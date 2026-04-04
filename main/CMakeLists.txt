idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        ble
        gpio
        http
        ota
        storage
        system_info
        wifi
)
