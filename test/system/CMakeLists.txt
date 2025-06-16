cmake_minimum_required(VERSION 3.16)

# Include unit-test-app components to get Unity framework
set(EXTRA_COMPONENT_DIRS 
    "$ENV{IDF_PATH}/tools/unit-test-app/components"
    "test_main"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(unit_test_app)
