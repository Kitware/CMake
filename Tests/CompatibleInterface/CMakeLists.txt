
cmake_minimum_required(VERSION 2.8)

project(CompatibleInterface)

include(GenerateExportHeader)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(iface1 empty.cpp)
set_property(TARGET iface1 APPEND PROPERTY
  COMPATIBLE_INTERFACE_BOOL
    BOOL_PROP1
    BOOL_PROP2
    BOOL_PROP3
    BOOL_PROP4
)
set_property(TARGET iface1 APPEND PROPERTY
  COMPATIBLE_INTERFACE_STRING
    STRING_PROP1
    STRING_PROP2
    STRING_PROP3
)

set_property(TARGET iface1 PROPERTY INTERFACE_BOOL_PROP1 ON)
set_property(TARGET iface1 PROPERTY INTERFACE_BOOL_PROP2 ON)
set_property(TARGET iface1 PROPERTY INTERFACE_STRING_PROP1 prop1)
set_property(TARGET iface1 PROPERTY INTERFACE_STRING_PROP2 prop2)

add_executable(CompatibleInterface main.cpp)
target_link_libraries(CompatibleInterface iface1)

set_property(TARGET CompatibleInterface PROPERTY BOOL_PROP2 ON)
set_property(TARGET CompatibleInterface PROPERTY BOOL_PROP3 ON)
set_property(TARGET CompatibleInterface PROPERTY STRING_PROP2 prop2)
set_property(TARGET CompatibleInterface PROPERTY STRING_PROP3 prop3)

target_compile_definitions(CompatibleInterface
  PRIVATE
    $<$<BOOL:$<TARGET_PROPERTY:BOOL_PROP1>>:BOOL_PROP1>
    $<$<BOOL:$<TARGET_PROPERTY:BOOL_PROP2>>:BOOL_PROP2>
    $<$<BOOL:$<TARGET_PROPERTY:BOOL_PROP3>>:BOOL_PROP3>
    $<$<STREQUAL:$<TARGET_PROPERTY:STRING_PROP1>,prop1>:STRING_PROP1>
    $<$<STREQUAL:$<TARGET_PROPERTY:STRING_PROP2>,prop2>:STRING_PROP2>
    $<$<STREQUAL:$<TARGET_PROPERTY:STRING_PROP3>,prop3>:STRING_PROP3>
)


add_library(iface2 SHARED iface2.cpp)
generate_export_header(iface2)

set_property(TARGET iface2 APPEND PROPERTY
  COMPATIBLE_INTERFACE_STRING
    Iface2_PROP
)

# For the LINK_LIBRARIES and related properties, we should not evaluate
# properties defined only in the interface - they should be implicitly zero
set_property(TARGET iface2
  APPEND PROPERTY
    LINK_INTERFACE_LIBRARIES $<$<BOOL:$<TARGET_PROPERTY:BOOL_PROP4>>:nonexistent>
)
target_link_libraries(CompatibleInterface iface2
      $<$<BOOL:$<TARGET_PROPERTY:Iface2_PROP>>:nonexistent>
)
# Test that this does not segfault:
target_compile_definitions(CompatibleInterface
  PRIVATE
    $<$<BOOL:$<TARGET_PROPERTY:Iface2_PROP>>:SOME_DEFINE>
)

# The COMPATIBLE_INTERFACE_* properties are only read from dependencies
# in the interface. Populating it on the CompatibleInterface target does
# not have any effect on the interpretation of the INTERFACE variants
# in dependencies.
set_property(TARGET iface1 PROPERTY
  INTERFACE_NON_RELEVANT_PROP ON
)
set_property(TARGET iface2 PROPERTY
  INTERFACE_NON_RELEVANT_PROP ON
)
set_property(TARGET CompatibleInterface APPEND PROPERTY
  COMPATIBLE_INTERFACE_BOOL
    NON_RELEVANT_PROP
)
