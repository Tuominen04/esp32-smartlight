cmake_minimum_required(VERSION 3.16)

# Include test main component
set(EXTRA_COMPONENT_DIRS
    "test_main"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(system_test_app)
