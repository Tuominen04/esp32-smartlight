idf_component_register(
  SRCS "ble_manager.c"
  INCLUDE_DIRS "." "../common"
  REQUIRES log freertos
  PRIV_REQUIRES storage wifi system_info
)

# Add bt as an optional dependency: included when CONFIG_BT_ENABLED=y,
# silently skipped when BT is disabled (e.g. test builds).
idf_component_optional_requires(PRIVATE bt)
