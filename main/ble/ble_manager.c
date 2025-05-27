#include "ble_manager.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gatt_common_api.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"

// Include other modules
#include "../device/device_info.h"
#include "../wifi/wifi_manager.h"
#include "../common/credentials.h"
#include "../storage/nvs_manager.h"

static const char *BLE_TAG = "BLUETOOTH";
static const char *GATTS_TAG = "GATTS";
static const char *GAP_TAG = "GAP";

static bool waiting_confrimation = false;
static bool confirmation_succesfull = false;
static char wifi_buffer[256] = {0};
static size_t wifi_buffer_len = 0;
static uint16_t device_info_char_handle = 0;

struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
};

static struct gatts_profile_inst gl_profile_tab[PROFILE_NUM];

// Advertising parameters
static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

// The advertising data
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = false,
    .min_interval = 0x20,
    .max_interval = 0x40,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(esp_service_uuid),
    .p_service_uuid = esp_service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// Scan response data (optional)
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(esp_service_uuid),
    .p_service_uuid = esp_service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static void ble_manager_gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    if (event == ESP_GATTS_REG_EVT) {
        // In the ESP_GATTS_REG_EVT case of ble_manager_gatts_profile_event_handler:
        ESP_LOGI(GATTS_TAG, "Creating service with UUID: %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            esp_service_uuid[15], esp_service_uuid[14], esp_service_uuid[13], esp_service_uuid[12], 
            esp_service_uuid[11], esp_service_uuid[10], esp_service_uuid[9], esp_service_uuid[8], 
            esp_service_uuid[7],esp_service_uuid[6], esp_service_uuid[5], esp_service_uuid[4], 
            esp_service_uuid[3], esp_service_uuid[2], esp_service_uuid[1], esp_service_uuid[0]);
        if (param->reg.status == ESP_GATT_OK) {
            gl_profile_tab[PROFILE_APP_IDX].gatts_if = gatts_if;
        } else {
            ESP_LOGE(GATTS_TAG, "Register app failed, app_id %04x, status %d", param->reg.app_id, param->reg.status);
            return;
        }
    }
    
    // If the gatts_if equals to profile A, call profile A's callback
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            if (gatts_if == ESP_GATT_IF_NONE || gatts_if == gl_profile_tab[idx].gatts_if) {
                if (gl_profile_tab[idx].gatts_cb) {
                    gl_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

// BLE GAP event handler
static void ble_manager_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        esp_ble_gap_start_advertising(&adv_params);
        break;
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        esp_ble_gap_start_advertising(&adv_params);
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        esp_ble_gap_start_advertising(&adv_params);
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GAP_TAG, "Advertising start failed");
        } else {
            ESP_LOGI(GAP_TAG, "Advertising started");
        }
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GAP_TAG, "Advertising stop failed");
        } else {
            ESP_LOGI(GAP_TAG, "Advertising stopped");
        }
        break;
    default:
        break;
    }
}

static void ble_manager_gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event) {
    case ESP_GATTS_REG_EVT:
        ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d", param->reg.status, param->reg.app_id);

        // Set up the device name
        esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(DEVICE_NAME);
        if (set_dev_name_ret) {
            ESP_LOGE(GATTS_TAG, "Set device name failed, error code = %x", set_dev_name_ret);
        }

        // Configure the advertisement data
        esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
        if (ret) {
            ESP_LOGE(GATTS_TAG, "Config adv data failed, error code = %x", ret);
        }
        
        // Configure the scan response data
        ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
        if (ret) {
            ESP_LOGE(GATTS_TAG, "Config scan response data failed, error code = %x", ret);
        }

        // Create the service
        esp_gatt_srvc_id_t service_id;
        service_id.id.inst_id = 0x00;
        service_id.id.uuid.len = ESP_UUID_LEN_128;
        memcpy(service_id.id.uuid.uuid.uuid128, esp_service_uuid, sizeof(esp_service_uuid));
        service_id.is_primary = true;
        
        esp_ble_gatts_create_service(gatts_if, &service_id, 8); /// Allow more handles for service + char + descriptor
        break;
        
        case ESP_GATTS_CREATE_EVT:
            ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d, service_handle %d", param->create.status, param->create.service_handle);
            gl_profile_tab[PROFILE_APP_IDX].service_handle = param->create.service_handle;
            
            // Start the service
            esp_ble_gatts_start_service(gl_profile_tab[PROFILE_APP_IDX].service_handle);
            
            // Add the WiFi credentials characteristic
            esp_bt_uuid_t char_uuid;
            char_uuid.len = ESP_UUID_LEN_128;
            memcpy(char_uuid.uuid.uuid128, wifi_characteristic_uuid, 16);
            
            uint8_t wifi_val[1] = {0};
            esp_attr_value_t wifi_val_struct = {
                .attr_max_len = 512,
                .attr_len = 1,
                .attr_value = wifi_val
            };
            
            esp_attr_control_t control = {
                .auto_rsp = ESP_GATT_AUTO_RSP
            };
            
            esp_err_t add_char_ret = esp_ble_gatts_add_char(
                gl_profile_tab[PROFILE_APP_IDX].service_handle, 
                &char_uuid,
                ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE,
                &wifi_val_struct, 
                &control
            );

            if (add_char_ret) {
                ESP_LOGE(GATTS_TAG, "Add char failed, error code = %x (%s)", add_char_ret, esp_err_to_name(add_char_ret));
            }
            break;

        case ESP_GATTS_ADD_CHAR_EVT:
            ESP_LOGI(GATTS_TAG, "ADD_CHAR_EVT, status %d, attr_handle %d, service_handle %d",
                    param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
            
            if (param->add_char.status != ESP_GATT_OK) {
                ESP_LOGE(GATTS_TAG, "Failed to add characteristic, error code = %d", param->add_char.status);
            } else {
                // Check which characteristic was added by comparing the char UUID
                uint8_t *char_uuid = param->add_char.char_uuid.uuid.uuid128;
                
                // Check if this is the WiFi credentials characteristic
                if (memcmp(char_uuid, wifi_characteristic_uuid, 16) == 0) {
                    gl_profile_tab[PROFILE_APP_IDX].char_handle = param->add_char.attr_handle;
                    ESP_LOGI(GATTS_TAG, "WiFi credentials characteristic added successfully");
                    
                    // Add the device info characteristic after a short delay
                    vTaskDelay(pdMS_TO_TICKS(100));
                    
                    // Add the device info characteristic (for sending IP & name back to app)
                    esp_bt_uuid_t device_info_char_uuid;
                    device_info_char_uuid.len = ESP_UUID_LEN_128;
                    memcpy(device_info_char_uuid.uuid.uuid128, device_info_characteristic_uuid, 16);
                    
                    uint8_t info_val[1] = {0};
                    esp_attr_value_t info_val_struct = {
                        .attr_max_len = 512,
                        .attr_len = 1,
                        .attr_value = info_val
                    };
                    
                    esp_attr_control_t info_control = {
                        .auto_rsp = ESP_GATT_AUTO_RSP
                    };
                    
                    ESP_LOGI(GATTS_TAG, "Adding device info characteristic with UUID: %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                        device_info_characteristic_uuid[15], device_info_characteristic_uuid[14], device_info_characteristic_uuid[13], device_info_characteristic_uuid[12], 
                        device_info_characteristic_uuid[11], device_info_characteristic_uuid[10], device_info_characteristic_uuid[9], device_info_characteristic_uuid[8], 
                        device_info_characteristic_uuid[7], device_info_characteristic_uuid[6], device_info_characteristic_uuid[5], device_info_characteristic_uuid[4], 
                        device_info_characteristic_uuid[3], device_info_characteristic_uuid[2], device_info_characteristic_uuid[1], device_info_characteristic_uuid[0]);
                    
                    esp_err_t add_info_char_ret = esp_ble_gatts_add_char(
                        gl_profile_tab[PROFILE_APP_IDX].service_handle, 
                        &device_info_char_uuid,
                        ESP_GATT_PERM_READ,
                        ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
                        &info_val_struct, 
                        &info_control
                    );

                    if (add_info_char_ret) {
                        ESP_LOGE(GATTS_TAG, "Add device info char failed, error code = %x (%s)", add_info_char_ret, esp_err_to_name(add_info_char_ret));
                    }
                } 
                // Check if this is the device info characteristic
                else if (memcmp(char_uuid, device_info_characteristic_uuid, 16) == 0) {
                    device_info_char_handle = param->add_char.attr_handle;
                    device_info_set_ble_handle(param->add_char.attr_handle);
                    ESP_LOGI(GATTS_TAG, "Device info characteristic added successfully");
                    
                    // Add CCCD descriptor for the device info characteristic
                    esp_bt_uuid_t cccd_uuid;
                    cccd_uuid.len = ESP_UUID_LEN_16;
                    cccd_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
                    
                    esp_attr_value_t cccd_val_struct = {
                        .attr_max_len = 2,
                        .attr_len = 2,
                        .attr_value = (uint8_t[]){0x00, 0x00}  // Initially notifications disabled
                    };
                    
                    esp_err_t add_cccd_ret = esp_ble_gatts_add_char_descr(
                        gl_profile_tab[PROFILE_APP_IDX].service_handle,
                        &cccd_uuid,
                        ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                        &cccd_val_struct,
                        NULL
                    );
                    
                    if (add_cccd_ret) {
                        ESP_LOGE(GATTS_TAG, "Add CCCD descriptor failed, error code = %x (%s)", 
                            add_cccd_ret, esp_err_to_name(add_cccd_ret));
                    } else {
                        ESP_LOGI(GATTS_TAG, "CCCD descriptor added successfully");
                            // Declaration of a variable to store the MAC address
                        const uint8_t *bt_mac;
                        // Retrieving the the MAC address of the Bluetooth interface
                        bt_mac = esp_bt_dev_get_address();
                        if (bt_mac == NULL) {
                            ESP_LOGE("BT", "Unable to retrieve the MAC address of the Bluetooth interface");
                            return;
                        }

                        // Writing the MAC address of the Bluetooth interface to the terminal
                        ESP_LOGI(BLE_TAG, "The Bluetooth interface MAC address: %02x:%02x:%02x:%02x:%02x:%02x",
                                bt_mac[0], bt_mac[1], bt_mac[2], bt_mac[3], bt_mac[4], bt_mac[5]);
                    }
                }
            }
            break;

    case ESP_GATTS_WRITE_EVT:
    
        if (param->write.handle == gl_profile_tab[PROFILE_APP_IDX].char_handle) {
            ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, handle = %d, len = %d", param->write.handle, param->write.len);
            
            // Log the received data in hex format for debugging
            ESP_LOG_BUFFER_HEX(GATTS_TAG, param->write.value, param->write.len);
            
            // Check if we have space in the buffer
            if (wifi_buffer_len + param->write.len < sizeof(wifi_buffer) - 1) {
                // Append the new data to our accumulation buffer
                memcpy(wifi_buffer + wifi_buffer_len, param->write.value, param->write.len);
                wifi_buffer_len += param->write.len;
                wifi_buffer[wifi_buffer_len] = 0; // Null terminate
                
                ESP_LOGI(GATTS_TAG, "Accumulated data");
                
                // Check if we have a complete JSON object
                if (wifi_buffer_len > 0 && wifi_buffer[0] == '{' && 
                    wifi_buffer[wifi_buffer_len-1] == '}') {

                    // Parse the JSON to check what type of message it is
                    cJSON *root = cJSON_Parse(wifi_buffer);
                    if (root) {
                    // Check if this is a confirmation message
                    cJSON *success_item = cJSON_GetObjectItem(root, "success");
                        if (success_item) {
                            // This is a confirmation message
                            ESP_LOGI(GATTS_TAG, "Received confirmation message: %s", 
                                success_item->valueint ? "SUCCESS" : "FAILURE");
                            
                            confirmation_succesfull = (success_item->valueint == 1);
                            waiting_confrimation = true;
                        } else {
                            // Check if it's WiFi credentials
                            cJSON *ssid_item = cJSON_GetObjectItem(root, "ssid");
                            cJSON *pass_item = cJSON_GetObjectItem(root, "password");
                            
                            if (ssid_item && pass_item) {
                                ESP_LOGI(GATTS_TAG, "Received complete WiFi credentials");
                                
                                // Copy to credentials buffer
                                wifi_manager_set_new_credentials(wifi_buffer);
                            } else {
                                ESP_LOGW(GATTS_TAG, "Received unknown JSON message");
                            }
                        }
                        
                        cJSON_Delete(root);
                    } else {
                        ESP_LOGW(GATTS_TAG, "Received invalid JSON data");
                    }
                    
                    // Reset accumulation buffer
                    memset(wifi_buffer, 0, sizeof(wifi_buffer));
                    wifi_buffer_len = 0;
                }
                
                // Always send a response
                esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
            } else {
                ESP_LOGE(GATTS_TAG, "Write buffer overflow, len=%d, buffer size=%d", 
                    wifi_buffer_len + param->write.len, sizeof(wifi_buffer) - 1);
                
                // Reset buffer on overflow
                memset(wifi_buffer, 0, sizeof(wifi_buffer));
                wifi_buffer_len = 0;
                
                // Still send a response but with an error
                esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_NO_RESOURCES, NULL);
            }
        } else {
            ESP_LOGE(GATTS_TAG, "Write to wrong handle (got %d, expected %d)",
                param->write.handle, gl_profile_tab[PROFILE_APP_IDX].char_handle);
            
            // Send response with error
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_INVALID_HANDLE, NULL);
        }
        break;

    case ESP_GATTS_CONNECT_EVT:
        ESP_LOGI(GATTS_TAG, "CONNECT_EVT, conn_id = %d", param->connect.conn_id);
        device_info_set_ble_info(gatts_if, param->connect.conn_id);
        gl_profile_tab[PROFILE_APP_IDX].conn_id = param->connect.conn_id;
        break;
        
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(GATTS_TAG, "DISCONNECT_EVT, reason = 0x%x", param->disconnect.reason);
        esp_ble_gap_start_advertising(&adv_params);
        break;
        
    default:
        break;
    }
}

void ble_manager_handle_device_info_confirmation(void*)
{
    while (1) {
        if (waiting_confrimation) {
            ESP_LOGI(BLE_TAG, "Handeling Confirmation");

            if (!confirmation_succesfull) {
                esp_err_t err = nvs_manager_delete_wifi_credentials();
                if (err == ESP_OK) {
                    wifi_manager_disconnect(); // Disconnect from current WiFi
                    ESP_LOGI(BLE_TAG, "Device info deleted after mobile application failed to receive device data.");                    
                    esp_ble_gap_start_advertising(&adv_params); // Restart BLE advertising to try again
                }
            } else {
                ESP_LOGI(BLE_TAG, "Mobile application confirmed successful receipt of device info.");
            }
            waiting_confrimation = false;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

esp_err_t ble_manager_init(void)
{
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    bt_cfg.controller_task_stack_size = 4096;
    esp_err_t ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(BLE_TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(BLE_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return ret;
    }
    
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(BLE_TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return ret;
    }
    
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(BLE_TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return ret;
    }

    // Register the BLE callbacks
    ret = esp_ble_gatts_register_callback(ble_manager_gatts_event_handler);
    if (ret) {
        ESP_LOGE(GATTS_TAG, "gatts register error, error code = %x", ret);
        return ret;
    }
    
    ret = esp_ble_gap_register_callback(ble_manager_gap_event_handler);
    if (ret) {
        ESP_LOGE(BLE_TAG, "gap register error, error code = %x", ret);
        return ret;
    }
    
    // Register the GATT profile
    ret = esp_ble_gatts_app_register(ESP_APP_ID);
    if (ret) {
        ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
        return ret;
    }

    // Initialize our profile
    gl_profile_tab[PROFILE_APP_IDX].gatts_cb = ble_manager_gatts_profile_event_handler;
    gl_profile_tab[PROFILE_APP_IDX].gatts_if = ESP_GATT_IF_NONE;

    return ESP_OK;
}

void ble_init(void) {
    ble_manager_init();
}