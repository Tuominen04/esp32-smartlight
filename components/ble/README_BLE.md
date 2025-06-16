## 📱 Device Setup via BLE
### Initial Configuration

Power on the device - BLE advertising starts automatically
1. Connect via BLE - Device appears as "ESP-C6-Light"
1. Send WiFi credentials to the WiFi characteristic:
   ```json
    {
       "ssid":"YourWiFiName",
       "password":"YourWiFiPassword"
    }
   ```
1. Receive device info from device info characteristic:
   ```json
    {
     "name":"ESP-C6-Light-A1B2C3D4",
     "id":"A1B2C3D4", 
     "ip":"192.168.1.100",
     "version":"0.0.1"
    }
   ```
1. Send confirmation back to WiFi characteristic:
   ```json
   {"success":1}
   ```


### BLE Service Details

* Service UUID: ``4b9131c3-c9c5-cc8f-9e45-b51f01c2af4f``
* WiFi Characteristic: ``a8261b36-07ea-f5b7-8846-e1363e48b5be`` (Read/Write)
* Device Info Characteristic: ``145f8763-1632-c09d-547c-bb6a451e20cf`` (Read/Notify)