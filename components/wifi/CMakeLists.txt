idf_component_register(
  SRCS "wifi_manager.c"
  INCLUDE_DIRS "." "../common"
  REQUIRES esp_wifi esp_event esp_netif log
  PRIV_REQUIRES storage cjson system_info
)
