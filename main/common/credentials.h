/**
 * @file credentials.h
 * @brief BLE service and characteristic UUIDs for device configuration.
 *
 * Contains the UUID definitions and GATT profile constants used for
 * BLE communication between the ESP device and mobile applications.
 */
#ifndef CREDENTIALS_H
#define CREDENTIALS_H

/** 
 * Primary BLE service UUID for device configuration.
 * Service UUID: 4b9131c3-c9c5-cc8f-9e45-b51f01c2af4f
 */
static uint8_t esp_service_uuid[16] = { 
    0x4f, 0xaf, 0xc2, 0x01, 0x1f, 0xb5, 0x45, 0x9e,
    0x8f, 0xcc, 0xc5, 0xc9, 0xc3, 0x31, 0x91, 0x4b 
};

/** 
 * BLE characteristic UUID for WiFi credentials exchange.
 * Characteristic UUID: a8261b36-07ea-f5b7-8846-e1363e48b5be
 * Used for receiving WiFi SSID and password from mobile app.
 */
static uint8_t wifi_characteristic_uuid[16] = {
    0xbe, 0xb5, 0x48, 0x3e, 0x36, 0xe1, 0x46, 0x88,
    0xb7, 0xf5, 0xea, 0x07, 0x36, 0x1b, 0x26, 0xa8
};

/** 
 * BLE characteristic UUID for device information exchange.
 * Characteristic UUID: 145f8763-1632-c09d-547c-bb6a451e20cf
 * Used for sending device IP, name, and version back to mobile app.
 */
static uint8_t device_info_characteristic_uuid[16] = {
    0xcf, 0x20, 0x1e, 0x45, 0x6a, 0xbb, 0x7c, 0x54,
    0x9d, 0xc0, 0x32, 0x16, 0x63, 0x87, 0x5f, 0x14  
};

/** Device name broadcasted in BLE advertisements. */
#define DEVICE_NAME "ESP-C6-Light"

/** Number of GATT profiles registered with the BLE stack. */
#define PROFILE_NUM 1

/** Index of the main application profile in the profile table. */
#define PROFILE_APP_IDX 0

/** Application ID used for GATT server registration. */
#define ESP_APP_ID 0
#endif /* CREDENTIALS_H */