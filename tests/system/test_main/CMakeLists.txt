# Test component CMakeLists.txt - Comprehensive version
idf_component_register(
    SRCS
        "test_system.c"
        "test_main.c"
    INCLUDE_DIRS
        "."
    REQUIRES
        unity
        nvs_flash
        driver
        esp_system
        log
)
