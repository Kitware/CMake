
include(GenerateExportHeader)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

cmake_policy(SET CMP0022 NEW)
add_library(cmp0022lib SHARED cmp0022lib.cpp)
generate_export_header(cmp0022lib)
add_library(cmp0022ifacelib SHARED cmp0022ifacelib.cpp)
generate_export_header(cmp0022ifacelib)
target_link_libraries(cmp0022lib LINK_PUBLIC cmp0022ifacelib)

assert_property(cmp0022lib LINK_INTERFACE_LIBRARIES "")
assert_property(cmp0022ifacelib LINK_INTERFACE_LIBRARIES "")
assert_property(cmp0022lib INTERFACE_LINK_LIBRARIES "cmp0022ifacelib")
assert_property(cmp0022ifacelib INTERFACE_LINK_LIBRARIES "")

add_executable(cmp0022exe cmp0022exe.cpp)
target_link_libraries(cmp0022exe cmp0022lib)

add_library(staticlib1 STATIC staticlib1.cpp)
generate_export_header(staticlib1)
add_library(staticlib2 STATIC staticlib2.cpp)
generate_export_header(staticlib2)
target_link_libraries(staticlib1 LINK_PUBLIC staticlib2)

add_executable(staticlib_exe staticlib_exe.cpp)
target_link_libraries(staticlib_exe staticlib1)
