
cmake_minimum_required(VERSION 2.8)

project(CompatibleInterface)

add_library(iface1 empty.cpp)
set_property(TARGET iface1 APPEND PROPERTY
  COMPATIBLE_INTERFACE_BOOL
    BOOL_PROP1
    BOOL_PROP2
    BOOL_PROP3
    BOOL_PROP4
)

set_property(TARGET iface1 PROPERTY INTERFACE_BOOL_PROP1 ON)
set_property(TARGET iface1 PROPERTY INTERFACE_BOOL_PROP2 ON)

add_executable(CompatibleInterface main.cpp)
target_link_libraries(CompatibleInterface iface1)

set_property(TARGET CompatibleInterface PROPERTY BOOL_PROP2 ON)
set_property(TARGET CompatibleInterface PROPERTY BOOL_PROP3 ON)

target_compile_definitions(CompatibleInterface
  PRIVATE
    $<$<BOOL:$<TARGET_PROPERTY:BOOL_PROP1>>:BOOL_PROP1>
    $<$<BOOL:$<TARGET_PROPERTY:BOOL_PROP2>>:BOOL_PROP2>
    $<$<BOOL:$<TARGET_PROPERTY:BOOL_PROP3>>:BOOL_PROP3>
)

add_library(iface2 SHARED empty.cpp)

# For the LINK_LIBRARIES and related properties, we should not evaluate
# properties defined only in the interface - they should be implicitly zero
set_property(TARGET iface2
  APPEND PROPERTY
    LINK_INTERFACE_LIBRARIES $<$<BOOL:$<TARGET_PROPERTY:BOOL_PROP4>>:nonexistant>
)
target_link_libraries(CompatibleInterface iface2)
