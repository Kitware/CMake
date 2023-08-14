set(CMAKE_C_FLAGS "-fsanitize=address" CACHE STRING "")
set(CMAKE_CXX_FLAGS "-fsanitize=address" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_fedora38_common.cmake")
