idf_component_register(
  SRCS "device_info.c"
  INCLUDE_DIRS "." "../common"
  REQUIRES nvs_flash log esp_wifi app_update bt
  PRIV_REQUIRES storage cjson
)
