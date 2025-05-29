# 🌐 HTTP API Reference
Once connected to WiFi, the device exposes a REST API:

## Light Control
``` bash
# Get light status
GET http://<device-ip>/light
Response: {"device":1,"state":"on"}

# Toggle light
PUT http://<device-ip>/toggle
Response: ON
```
OTA Updates

```bash
# Get firmware info
GET http://<device-ip>/ota/firmware-info
Response: {
  "version": "0.0.1",
  "project_name": "light_client",
  "app_elf_sha256": "abc123...",
  "date": "Jan 29 2025",
  "time": "15:30:45",
  "ota_in_progress": false
}

# Start OTA update
POST http://<device-ip>/ota/update
Content-Type: application/json
Body: {"url":"http://example.com/firmware.bin"}

# Check OTA progress
GET http://<device-ip>/ota/progress
Response: {
  "in_progress": true,
  "progress": 45.2,
  "status": "Downloading..."
}
```