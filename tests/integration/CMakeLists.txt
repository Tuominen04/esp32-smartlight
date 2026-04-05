cmake_minimum_required(VERSION 3.16)

# Include project components and test main
set(EXTRA_COMPONENT_DIRS
  "../../components"
  "test_main"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(integration_test_app)
