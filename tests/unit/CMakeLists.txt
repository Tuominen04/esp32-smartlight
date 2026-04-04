cmake_minimum_required(VERSION 3.16)

# Include Unity test framework from ESP-IDF
set(EXTRA_COMPONENT_DIRS
    "$ENV{IDF_PATH}/tools/unit-test-app/components"
    "../../components"
    "test_main"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(unit_test_app)
