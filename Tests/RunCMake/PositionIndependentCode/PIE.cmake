
cmake_policy(SET CMP0083 NEW)

include(CheckPIESupported)
check_pie_supported()

add_executable (pie_on main.cpp)
set_property(TARGET pie_on PROPERTY POSITION_INDEPENDENT_CODE ON)

add_executable (pie_off main.cpp)
set_property(TARGET pie_off PROPERTY POSITION_INDEPENDENT_CODE OFF)


# generate file holding paths to executables
file (GENERATE OUTPUT "${CMAKE_BINARY_DIR}/$<CONFIG>/PIE_config.cmake"
               CONTENT
[==[
include ("${RunCMake_TEST_SOURCE_DIR}/PIE_validator.cmake")

set (pie_on "$<TARGET_FILE:pie_on>")
set (pie_off "$<TARGET_FILE:pie_off>")
]==])
