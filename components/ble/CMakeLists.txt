idf_component_register(
  SRCS "ble_manager.c"
  INCLUDE_DIRS "." "../common"
  REQUIRES bt log freertos
  PRIV_REQUIRES storage wifi system_info
)
