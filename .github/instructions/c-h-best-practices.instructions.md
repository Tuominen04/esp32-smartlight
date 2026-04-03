# C Source/Headers Best Practices for ESP32 Smart Light

Apply these rules when authoring or updating C and header files in this repository:

1. Indent with 2 spaces (no tabs) for all source and header files.
2. Use include guards in every header:
   - `#ifndef COMPONENTS_WIFI_WIFI_MANAGER_H`
   - `#define COMPONENTS_WIFI_WIFI_MANAGER_H`
   - `#endif // COMPONENTS_WIFI_WIFI_MANAGER_H`
3. Keep API declarations in `.h`, implementation in `.c`; avoid internal state leakage.
4. Document each public function with purpose, inputs, outputs, and error handling.
5. Check arguments early (`if (ptr == NULL) return ESP_ERR_INVALID_ARG;`).
6. Avoid global mutable variables; pass context structs as function arguments.
7. Keep functions concise (target < 80 lines). Break complex logic into helpers.
8. Follow snake_case naming for functions/variables, UPPER_SNAKE_CASE for macros.
9. Log errors and key states with `ESP_LOGI/W/E/D` in implementation functions.
10. Include unit tests in `tests/unit` for logic/path changes.
