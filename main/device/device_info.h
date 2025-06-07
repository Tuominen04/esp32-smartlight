#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_ota_ops.h"

// Only include BLE headers if BLE is enabled
#ifdef CONFIG_BT_ENABLED
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"
#endif

/**
 * @brief Unique device number or instance identifier.
 * Used internally for addressing or naming if needed.
 */
#define DEVICE_NUMBER 1

/**
 * @brief Save device information and WiFi credentials to NVS.
 *
 * This function creates a unique device name and ID using the MAC address.
 * If valid device info already exists in NVS, it skips saving again.
 *
 * @param ssid      WiFi SSID to save.
 * @param password  WiFi password to save.
 */
void device_manager_save_device_info(const char* ssid, const char* password);

/**
 * @brief Send device information to the mobile app via BLE.
 *
 * Builds a JSON payload with device name, ID, IP address, and firmware version,
 * then updates the BLE characteristic and sends a notification if connected.
 */
#ifdef CONFIG_BT_ENABLED
void send_device_info_via_ble(void);
#endif

/**
 * @brief Get information about the currently running firmware.
 *
 * Returns a dynamically allocated structure containing metadata such as
 * firmware version, project name, and compile time.
 *
 * @return Pointer to an `esp_app_desc_t` structure, or NULL on failure.
 *         Caller is responsible for freeing the memory.
 */
esp_app_desc_t* get_firmware_info(void);

#ifdef CONFIG_BT_ENABLED
/**
 * @brief Set BLE GATT interface and connection ID.
 *
 * Called by the BLE module upon device connection to set up the necessary BLE context.
 *
 * @param gatts_if   BLE GATT interface.
 * @param conn_id    BLE connection ID.
 */
void device_info_set_ble_info(esp_gatt_if_t gatts_if, uint16_t conn_id);

/**
 * @brief Set the BLE characteristic handle used to send device information.
 *
 * Called by the BLE module after the characteristic is created.
 *
 * @param handle  BLE GATT characteristic handle.
 */
void device_info_set_ble_handle(uint16_t handle);

/**
 * @brief Callback type for setting the BLE characteristic handle externally.
 */
typedef void (*set_device_info_handle_cb_t)(uint16_t handle);
#endif // CONFIG_BT_ENABLED

#endif // DEVICE_INFO_H
