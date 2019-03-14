
# create reference to detect default : PIE or not
add_executable (cmp0083_ref main.cpp)


set (CMAKE_POSITION_INDEPENDENT_CODE ON)

cmake_policy(SET CMP0083 NEW)
include(CheckPIESupported)
check_pie_supported()
add_executable (cmp0083_new_pie main.cpp)


cmake_policy(SET CMP0083 OLD)
add_executable (cmp0083_old_pie main.cpp)


set (CMAKE_POSITION_INDEPENDENT_CODE OFF)

cmake_policy(SET CMP0083 NEW)
add_executable (cmp0083_new_no_pie main.cpp)


cmake_policy(SET CMP0083 OLD)
add_executable (cmp0083_old_no_pie main.cpp)

# high-level targets
add_custom_target(cmp0083_new)
add_dependencies(cmp0083_new cmp0083_ref cmp0083_new_pie cmp0083_new_no_pie)

# high-level targets
add_custom_target(cmp0083_old)
add_dependencies(cmp0083_old cmp0083_ref cmp0083_old_pie cmp0083_old_no_pie)


# generate file holding paths to executables
file (GENERATE OUTPUT "${CMAKE_BINARY_DIR}/$<CONFIG>/CMP0083_config.cmake"
               CONTENT
[==[
include ("${RunCMake_TEST_SOURCE_DIR}/PIE_validator.cmake")

set (cmp0083_ref "$<TARGET_FILE:cmp0083_ref>")
set (cmp0083_new_pie "$<TARGET_FILE:cmp0083_new_pie>")
set (cmp0083_old_pie "$<TARGET_FILE:cmp0083_old_pie>")
set (cmp0083_new_no_pie "$<TARGET_FILE:cmp0083_new_no_pie>")
set (cmp0083_old_no_pie "$<TARGET_FILE:cmp0083_old_no_pie>")
]==])
