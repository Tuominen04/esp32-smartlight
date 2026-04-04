idf_component_register(
  SRCS "http_server.c"
  INCLUDE_DIRS "." "../common"
  REQUIRES esp_http_server log
  PRIV_REQUIRES gpio ota system_info cjson
)
