set(CMAKE_SYSTEM_NAME Android)

# This test doesn't require the NDK, so inhibit CMake's NDK handling code.
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_FIND_ROOT_PATH "${CMAKE_SOURCE_DIR}/android_root")
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
