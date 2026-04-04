idf_component_register(
  SRCS "ota_manager.c"
  INCLUDE_DIRS "." "../common"
  REQUIRES app_update esp_http_client esp_http_server esp_https_ota log esp_timer
  PRIV_REQUIRES cjson
)
