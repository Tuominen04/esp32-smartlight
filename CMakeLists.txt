cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(EXTRA_COMPONENT_DIRS "tests/unit/test_main" "tests/system/test_main")
project(light_client VERSION 0.0.1)
