cmake_minimum_required(VERSION 3.16)

# Unit tests are self-contained and do not import hardware-dependent components.
# Only the test_main component (which includes common_defs.h as an include dir)
# is registered here. Target: linux, so these tests run directly in CI.
set(EXTRA_COMPONENT_DIRS
  "test_main"
)

# Restrict to only the components we use — prevents hardware-only components
# (e.g. esp_hal_ana_cmpr) from being pulled in when targeting linux.
set(COMPONENTS test_main unity)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(unit_test_app)
