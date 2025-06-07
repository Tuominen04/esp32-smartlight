# Test component test/test_main/CMakeLists.txt
idf_component_register(
    SRCS 
        "test_main.c"
        "test_nvs_manager.c"
        "test_wifi_manager.c" 
        "test_gpio_control.c"
        "test_device_info.c"
        # Add only the source files we need for testing, not the full main
        "../../main/storage/nvs_manager.c"
        "../../main/gpio/gpio_control.c"
        "../../main/device/device_info.c"
        "../../main/wifi/wifi_manager.c"

    INCLUDE_DIRS 
        "."
        "../../main"
        "../../main/storage"
        "../../main/wifi"
        "../../main/gpio"
        "../../main/device"
        "../../main/common"
        "../../main/http"
        "../../main/ota"
    REQUIRES 
        unity
        app_update
        bt
        # esp_common
        esp_driver_gpio
        esp_event
        # esp_http_client 
        esp_https_ota
        esp_netif
        esp_timer
        esp_wifi
        freertos
        json
        nvs_flash
    )
    