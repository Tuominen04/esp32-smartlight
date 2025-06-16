idf_component_register(
    SRCS 
        "test_main.c"
        "test_nvs_manager.c"
        "test_wifi_manager.c" 
        "test_gpio_control.c"
        "test_device_info.c"
        # Add only the source files we need for testing, not the full main
        "../../../components/gpio/gpio_control.c"
        "../../../components/storage/nvs_manager.c"
        "../../../components/system_info/device_info.c"
        "../../../components/wifi/wifi_manager.c"

    INCLUDE_DIRS 
        "."
        "../../../components"
        "../../../components/common"
        "../../../components/gpio"
        "../../../components/http"
        "../../../components/ota"
        "../../../components/storage"
        "../../../components/system_info"
        "../../../components/wifi"

    REQUIRES 
        unity
        app_update
        bt
        esp_driver_gpio
        esp_event
        esp_https_ota
        esp_netif
        esp_timer
        esp_wifi
        freertos
        json
        nvs_flash
    )
    