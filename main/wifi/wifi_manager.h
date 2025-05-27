#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_event.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define MAXIMUM_RETRY 5

// Callback function types
typedef void (*wifi_connected_cb_t)(void);
typedef void (*wifi_disconnected_cb_t)(void);

// Public functions
esp_err_t wifi_manager_init(void);
esp_err_t wifi_manager_connect(const char *ssid, const char *password);
bool wifi_manager_is_connected(void);
esp_err_t wifi_manager_disconnect(void);
esp_err_t wifi_manager_get_saved_credentials(char* out_ssid, size_t ssid_buf_size, char* out_password, size_t password_buf_size);

// Legacy functions (for compatibility with existing code)
void wifi_init(void);
void wifi_init_sta(const char *ssid, const char *pass);
void wifi_connect(const char *ssid, const char *pass);

// WiFi credentials management
void wifi_manager_set_new_credentials(const char *json_credentials);
bool wifi_manager_has_new_credentials(void);
void wifi_manager_handle_new_credentials_task(void *pvParameters);

// Callback management
void wifi_manager_set_callbacks(wifi_connected_cb_t on_connect, wifi_disconnected_cb_t on_disconnect);

// Event group access
EventGroupHandle_t wifi_manager_get_event_group(void);

#endif // WIFI_MANAGER_H