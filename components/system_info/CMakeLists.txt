idf_component_register(
  SRCS "device_info.c"
  INCLUDE_DIRS "." "../common"
  REQUIRES nvs_flash log esp_wifi app_update
  PRIV_REQUIRES storage cjson
)

# Add bt as an optional dependency: included when CONFIG_BT_ENABLED=y,
# silently skipped when BT is disabled (e.g. test builds).
idf_component_optional_requires(PUBLIC bt)
