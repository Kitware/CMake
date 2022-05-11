cmake_policy(SET CMP0028 NEW)

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

# Test adding unquoted genex with ';' to LINK_LIBRARIES and INTERFACE_LINK_LIBRARIES.
target_link_libraries(cmp0022lib
  PUBLIC $<0:imp::missing1;imp::missing2>
  PRIVATE $<0:imp::missing3;imp::missing4>
  INTERFACE $<0:imp::missing5;imp::missing6>
  )
assert_property(cmp0022lib INTERFACE_LINK_LIBRARIES "cmp0022ifacelib;$<0:imp::missing1;imp::missing2>;$<0:imp::missing5;imp::missing6>")
assert_property(cmp0022lib LINK_LIBRARIES "cmp0022ifacelib;$<0:imp::missing1;imp::missing2>;$<0:imp::missing3;imp::missing4>")

add_library(staticlib1 STATIC staticlib1.cpp)
generate_export_header(staticlib1)
add_library(staticlib2 STATIC staticlib2.cpp)
generate_export_header(staticlib2)
target_link_libraries(staticlib1 LINK_PUBLIC staticlib2)

# Test adding LINK_ONLY to each of multiple specified libraries.
add_library(staticlib2iface1 INTERFACE)
add_library(staticlib2iface2 INTERFACE)
target_compile_definitions(staticlib2iface1 INTERFACE STATICLIB2_IFACE_1)
target_compile_definitions(staticlib2iface2 INTERFACE STATICLIB2_IFACE_2)
target_link_libraries(staticlib2 PRIVATE staticlib2iface1 staticlib2iface2)

# Test adding unquoted genex with ';' to LINK_LIBRARIES and INTERFACE_LINK_LIBRARIES.
target_link_libraries(staticlib2
  PUBLIC $<0:imp::missing1;imp::missing2>
  PRIVATE $<0:imp::missing3;imp::missing4>
  INTERFACE $<0:imp::missing5;imp::missing6>
  )
assert_property(staticlib2 INTERFACE_LINK_LIBRARIES "$<LINK_ONLY:staticlib2iface1>;$<LINK_ONLY:staticlib2iface2>;$<0:imp::missing1;imp::missing2>;$<LINK_ONLY:$<0:imp::missing3;imp::missing4>>;$<0:imp::missing5;imp::missing6>")
assert_property(staticlib2 LINK_LIBRARIES "staticlib2iface1;staticlib2iface2;$<0:imp::missing1;imp::missing2>;$<0:imp::missing3;imp::missing4>")

# Try adding a private link item to be propagated out of a static lib.
set(private_link "")
if (CMAKE_CXX_COMPILER_ID MATCHES GNU OR CMAKE_CXX_COMPILER_ID MATCHES Clang OR CMAKE_CXX_COMPILER_ID MATCHES LCC)
  if (CMAKE_SYSTEM_NAME STREQUAL "SunOS")
    set(private_link "-Wl,-V")
  else()
    set(private_link "-Wl,-v")
  endif()
endif()
if(private_link)
  target_link_libraries(staticlib1 LINK_PRIVATE "${private_link}")
endif()

add_executable(staticlib_exe staticlib_exe.cpp)
target_link_libraries(staticlib_exe staticlib1)

add_library(onlyplainlib1 SHARED onlyplainlib1.cpp)
add_library(onlyplainlib2 SHARED onlyplainlib2.cpp)
target_link_libraries(onlyplainlib2 onlyplainlib1)

add_executable(onlyplainlib_user onlyplainlib_user.cpp)
target_link_libraries(onlyplainlib_user onlyplainlib2)
