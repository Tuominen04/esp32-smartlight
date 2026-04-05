idf_component_register(
  SRCS
    "test_main.c"
    "test_common_defs.c"
    "test_credential_validation.c"

  INCLUDE_DIRS
    "."
    "../../../components/common"

  REQUIRES
    unity
)
