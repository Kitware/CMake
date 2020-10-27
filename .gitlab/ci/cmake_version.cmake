get_filename_component(CMake_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
include("${CMake_SOURCE_DIR}/Source/CMakeVersion.cmake")
message(STATUS ${CMake_VERSION})
