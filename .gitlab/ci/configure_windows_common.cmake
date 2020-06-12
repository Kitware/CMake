set(BUILD_QtDialog ON CACHE BOOL "")
set(CMAKE_PREFIX_PATH "$ENV{CI_PROJECT_DIR}/.gitlab/qt" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_common.cmake")
