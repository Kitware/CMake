set(CMAKE_C_FLAGS "-fsanitize=address" CACHE STRING "")
set(CMAKE_CXX_FLAGS "-fsanitize=address" CACHE STRING "")
set(CMake_QT_MAJOR_VERSION "5" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_fedora42_common.cmake")
