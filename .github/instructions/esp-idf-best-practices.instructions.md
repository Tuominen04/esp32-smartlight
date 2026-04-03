# ESP-IDF Best Practices for ESP32 Smart Light

Apply these rules when working with ESP-IDF APIs and platform behavior:

1. Return `esp_err_t` for API-like functions; use standard codes (`ESP_OK`, `ESP_FAIL`, `ESP_ERR_INVALID_ARG`).
2. Initialize core subsystems before use (`esp_netif_init`, `esp_event_loop_create_default`, `esp_wifi_init`).
3. Use event-driven patterns for WiFi/BLE/state changes; avoid blocking loops on main execution path.
4. Check return values for every ESP-IDF call and log errors via `ESP_LOGE` before returning.
5. Handle NVS open errors, including migration path on `ESP_ERR_NVS_NO_FREE_PAGES` and `ESP_ERR_NVS_NEW_VERSION_FOUND`.
6. Use resource cleanup on failure (`esp_wifi_stop`, `esp_wifi_deinit`, `esp_netif_deinit`, etc.).
7. Keep log statements non-sensitive; never write credentials or PII to logs.
8. Use `esp_timer` or FreeRTOS timers instead of `vTaskDelay` for precise timing when relevant.
9. Use menuconfig defaults for tunable values and avoid hardcoded magic numbers.
10. Add system-level documentation in `docs/` when implementation behavior is changed (network, OTA, power, etc.).
