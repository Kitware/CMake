
cmake_minimum_required(VERSION 3.0)

project(CompatibleInterface)

include(GenerateExportHeader)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(iface1 INTERFACE)
set_property(TARGET iface1 APPEND PROPERTY
  COMPATIBLE_INTERFACE_BOOL
    BOOL_PROP1
    BOOL_PROP2
    BOOL_PROP3
    BOOL_PROP4
    BOOL_PROP5
)
set_property(TARGET iface1 APPEND PROPERTY
  COMPATIBLE_INTERFACE_STRING
    STRING_PROP1
    STRING_PROP2
    STRING_PROP3
    STRING_PROP4
)
set_property(TARGET iface1 APPEND PROPERTY
  COMPATIBLE_INTERFACE_NUMBER_MIN
    NUMBER_MIN_PROP1
    NUMBER_MIN_PROP2
    NUMBER_MIN_PROP3
    NUMBER_MIN_PROP4
    NUMBER_MIN_PROP5
    NUMBER_MIN_PROP6
)
set_property(TARGET iface1 APPEND PROPERTY
  COMPATIBLE_INTERFACE_NUMBER_MAX
    NUMBER_MAX_PROP1
    NUMBER_MAX_PROP2
    NUMBER_MAX_PROP3
    NUMBER_MAX_PROP4
)

set(CMAKE_DEBUG_TARGET_PROPERTIES
  BOOL_PROP1 BOOL_PROP2 BOOL_PROP3 BOOL_PROP4 BOOL_PROP5
  STRING_PROP1 STRING_PROP2 STRING_PROP3 STRING_PROP4
  NUMBER_MIN_PROP1 NUMBER_MIN_PROP2 NUMBER_MIN_PROP3 NUMBER_MIN_PROP4 NUMBER_MIN_PROP5 NUMBER_MIN_PROP6
  NUMBER_MAX_PROP1 NUMBER_MAX_PROP2 NUMBER_MAX_PROP3 NUMBER_MAX_PROP4
)

set_property(TARGET iface1 PROPERTY INTERFACE_BOOL_PROP1 ON)
set_property(TARGET iface1 PROPERTY INTERFACE_BOOL_PROP2 ON)
set_property(TARGET iface1 PROPERTY INTERFACE_BOOL_PROP5 ON)
set_property(TARGET iface1 PROPERTY INTERFACE_STRING_PROP1 prop1)
set_property(TARGET iface1 PROPERTY INTERFACE_STRING_PROP2 prop2)
set_property(TARGET iface1 PROPERTY INTERFACE_STRING_PROP4 prop4)
set_property(TARGET iface1 PROPERTY INTERFACE_NUMBER_MIN_PROP1 100)
set_property(TARGET iface1 PROPERTY INTERFACE_NUMBER_MIN_PROP2 200)
set_property(TARGET iface1 PROPERTY INTERFACE_NUMBER_MIN_PROP3 0x10)
set_property(TARGET iface1 PROPERTY INTERFACE_NUMBER_MIN_PROP4 0x10)
set_property(TARGET iface1 PROPERTY INTERFACE_NUMBER_MIN_PROP5 5)
set_property(TARGET iface1 PROPERTY INTERFACE_NUMBER_MIN_PROP6 6)
set_property(TARGET iface1 PROPERTY INTERFACE_NUMBER_MAX_PROP1 100)
set_property(TARGET iface1 PROPERTY INTERFACE_NUMBER_MAX_PROP2 200)
set_property(TARGET iface1 PROPERTY INTERFACE_NUMBER_MAX_PROP3 3)
set_property(TARGET iface1 PROPERTY INTERFACE_NUMBER_MAX_PROP4 4)

add_executable(CompatibleInterface main.cpp)
target_link_libraries(CompatibleInterface iface1)

add_library(foo STATIC foo.cpp)
add_library(bar SHARED bar.cpp)
set_property(TARGET foo APPEND PROPERTY COMPATIBLE_INTERFACE_BOOL SOMEPROP)
set_property(TARGET foo PROPERTY INTERFACE_SOMEPROP ON)
# Use LINK_ONLY to suppress usage requirements and allow the check to pass.
set_property(TARGET bar PROPERTY INTERFACE_LINK_LIBRARIES $<LINK_ONLY:foo>)
set_property(TARGET CompatibleInterface PROPERTY SOMEPROP OFF)
target_link_libraries(CompatibleInterface bar)

set_property(TARGET CompatibleInterface PROPERTY BOOL_PROP2 ON)
set_property(TARGET CompatibleInterface PROPERTY BOOL_PROP3 ON)
set_property(TARGET CompatibleInterface PROPERTY STRING_PROP2 prop2)
set_property(TARGET CompatibleInterface PROPERTY STRING_PROP3 prop3)
set_property(TARGET CompatibleInterface PROPERTY NUMBER_MIN_PROP1 50)
set_property(TARGET CompatibleInterface PROPERTY NUMBER_MIN_PROP2 250)
set_property(TARGET CompatibleInterface PROPERTY NUMBER_MIN_PROP3 0xa)
set_property(TARGET CompatibleInterface PROPERTY NUMBER_MIN_PROP4 0x1A)
set_property(TARGET CompatibleInterface PROPERTY NUMBER_MAX_PROP1 50)
set_property(TARGET CompatibleInterface PROPERTY NUMBER_MAX_PROP2 250)

target_compile_definitions(CompatibleInterface
  PRIVATE
    $<$<BOOL:$<TARGET_PROPERTY:BOOL_PROP1>>:BOOL_PROP1>
    $<$<BOOL:$<TARGET_PROPERTY:BOOL_PROP2>>:BOOL_PROP2>
    $<$<BOOL:$<TARGET_PROPERTY:BOOL_PROP3>>:BOOL_PROP3>
    $<$<STREQUAL:$<TARGET_PROPERTY:STRING_PROP1>,prop1>:STRING_PROP1>
    $<$<STREQUAL:$<TARGET_PROPERTY:STRING_PROP2>,prop2>:STRING_PROP2>
    $<$<STREQUAL:$<TARGET_PROPERTY:STRING_PROP3>,prop3>:STRING_PROP3>
    $<$<STREQUAL:$<TARGET_PROPERTY:NUMBER_MIN_PROP1>,50>:NUMBER_MIN_PROP1=50>
    $<$<STREQUAL:$<TARGET_PROPERTY:NUMBER_MIN_PROP2>,200>:NUMBER_MIN_PROP2=200>
    $<$<EQUAL:$<TARGET_PROPERTY:NUMBER_MIN_PROP3>,0xA>:NUMBER_MIN_PROP3=0xA>
    $<$<STREQUAL:$<TARGET_PROPERTY:NUMBER_MIN_PROP4>,0x10>:NUMBER_MIN_PROP4=0x10>
    $<$<STREQUAL:$<TARGET_PROPERTY:NUMBER_MIN_PROP5>,5>:NUMBER_MIN_PROP5=5>
    $<$<STREQUAL:$<TARGET_PROPERTY:NUMBER_MAX_PROP1>,100>:NUMBER_MAX_PROP1=100>
    $<$<STREQUAL:$<TARGET_PROPERTY:NUMBER_MAX_PROP2>,250>:NUMBER_MAX_PROP2=250>
    $<$<STREQUAL:$<TARGET_PROPERTY:NUMBER_MAX_PROP3>,3>:NUMBER_MAX_PROP3=3>

    # Static libraries compute COMPATIBLE_INTERFACE_ properties transitively.
    $<$<BOOL:$<TARGET_PROPERTY:static1,BOOL_PROP1>>:STATIC1_BOOL_PROP1>
    $<$<STREQUAL:$<TARGET_PROPERTY:static1,STRING_PROP1>,prop1>:STATIC1_STRING_PROP1>
    $<$<EQUAL:$<TARGET_PROPERTY:static1,NUMBER_MAX_PROP3>,3>:STATIC1_NUMBER_MAX_PROP3>
    $<$<EQUAL:$<TARGET_PROPERTY:static1,NUMBER_MIN_PROP5>,5>:STATIC1_NUMBER_MIN_PROP5>

    # Object libraries do not compute COMPATIBLE_INTERFACE_ properties transitively.
    $<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:object1,BOOL_PROP1>,>>:OBJECT1_BOOL_PROP1>
    $<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:object1,STRING_PROP1>,>>:OBJECT1_STRING_PROP1>
    $<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:object1,NUMBER_MAX_PROP3>,>>:OBJECT1_NUMBER_MAX_PROP3>
    $<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:object1,NUMBER_MIN_PROP5>,>>:OBJECT1_NUMBER_MIN_PROP5>

    # Interface libraries do not compute COMPATIBLE_INTERFACE_ properties transitively.
    $<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:iface3,BOOL_PROP1>,>>:IFACE3_BOOL_PROP1>
    $<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:iface3,STRING_PROP1>,>>:IFACE3_STRING_PROP1>
    $<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:iface3,NUMBER_MAX_PROP3>,>>:IFACE3_NUMBER_MAX_PROP3>
    $<$<NOT:$<STREQUAL:$<TARGET_PROPERTY:iface3,NUMBER_MIN_PROP5>,>>:IFACE3_NUMBER_MIN_PROP5>

    # Static libraries compute COMPATIBLE_INTERFACE_ properties transitively.
    $<$<BOOL:$<TARGET_PROPERTY:static1,BOOL_PROP5>>:STATIC1_BOOL_PROP5>
    $<$<STREQUAL:$<TARGET_PROPERTY:static1,STRING_PROP4>,prop4>:STATIC1_STRING_PROP4>
    $<$<EQUAL:$<TARGET_PROPERTY:static1,NUMBER_MIN_PROP6>,6>:STATIC1_NUMBER_MIN_PROP6>
    $<$<EQUAL:$<TARGET_PROPERTY:static1,NUMBER_MAX_PROP4>,4>:STATIC1_NUMBER_MAX_PROP4>

    # Object libraries do not compute COMPATIBLE_INTERFACE_ properties transitively,
    # but can have properties set on them.
    $<$<BOOL:$<TARGET_PROPERTY:object1,BOOL_PROP5>>:OBJECT1_BOOL_PROP5>
    $<$<STREQUAL:$<TARGET_PROPERTY:object1,STRING_PROP4>,prop4>:OBJECT1_STRING_PROP4>
    $<$<EQUAL:$<TARGET_PROPERTY:object1,NUMBER_MIN_PROP6>,7>:OBJECT1_NUMBER_MIN_PROP6>
    $<$<EQUAL:$<TARGET_PROPERTY:object1,NUMBER_MAX_PROP4>,1>:OBJECT1_NUMBER_MAX_PROP4>

    # Interface libraries do not compute COMPATIBLE_INTERFACE_ properties transitively,
    # but can have properties set on them.
    $<$<BOOL:$<TARGET_PROPERTY:iface3,BOOL_PROP5>>:IFACE3_BOOL_PROP5>
    $<$<STREQUAL:$<TARGET_PROPERTY:iface3,STRING_PROP4>,prop4>:IFACE3_STRING_PROP4>
    $<$<EQUAL:$<TARGET_PROPERTY:iface3,NUMBER_MIN_PROP6>,7>:IFACE3_NUMBER_MIN_PROP6>
    $<$<EQUAL:$<TARGET_PROPERTY:iface3,NUMBER_MAX_PROP4>,1>:IFACE3_NUMBER_MAX_PROP4>
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

add_library(static1 STATIC foo.cpp)
set_property(TARGET static1 PROPERTY BOOL_PROP5 ON)
set_property(TARGET static1 PROPERTY STRING_PROP4 prop4)
set_property(TARGET static1 PROPERTY NUMBER_MIN_PROP6 7)
set_property(TARGET static1 PROPERTY NUMBER_MAX_PROP4 1)
target_link_libraries(static1 PUBLIC iface1)

add_library(object1 OBJECT foo.cpp)
set_property(TARGET object1 PROPERTY BOOL_PROP5 ON)
set_property(TARGET object1 PROPERTY STRING_PROP4 prop4)
set_property(TARGET object1 PROPERTY NUMBER_MIN_PROP6 7)
set_property(TARGET object1 PROPERTY NUMBER_MAX_PROP4 1)
target_link_libraries(object1 PUBLIC iface1)

add_library(iface3 INTERFACE)
set_property(TARGET iface3 PROPERTY BOOL_PROP5 ON)
set_property(TARGET iface3 PROPERTY STRING_PROP4 prop4)
set_property(TARGET iface3 PROPERTY NUMBER_MIN_PROP6 7)
set_property(TARGET iface3 PROPERTY NUMBER_MAX_PROP4 1)
target_link_libraries(iface3 INTERFACE iface1)

# Test COMPATIBLE_INTERFACE_* property evaluation outside of usage requirements.
add_custom_target(check ALL VERBATIM
  COMMAND CompatibleInterface
  # expect     actual
    "1"        "$<TARGET_PROPERTY:CompatibleInterface,BOOL_PROP1>"
    "prop1"    "$<TARGET_PROPERTY:CompatibleInterface,STRING_PROP1>"
    "3"        "$<TARGET_PROPERTY:CompatibleInterface,NUMBER_MAX_PROP3>"
    "5"        "$<TARGET_PROPERTY:CompatibleInterface,NUMBER_MIN_PROP5>"

    "1"        "$<TARGET_PROPERTY:static1,BOOL_PROP1>"
    "prop1"    "$<TARGET_PROPERTY:static1,STRING_PROP1>"
    "3"        "$<TARGET_PROPERTY:static1,NUMBER_MAX_PROP3>"
    "5"        "$<TARGET_PROPERTY:static1,NUMBER_MIN_PROP5>"

    ""         "$<TARGET_PROPERTY:object1,BOOL_PROP1>"
    ""         "$<TARGET_PROPERTY:object1,STRING_PROP1>"
    ""         "$<TARGET_PROPERTY:object1,NUMBER_MAX_PROP3>"
    ""         "$<TARGET_PROPERTY:object1,NUMBER_MIN_PROP5>"

    ""         "$<TARGET_PROPERTY:iface3,BOOL_PROP1>"
    ""         "$<TARGET_PROPERTY:iface3,STRING_PROP1>"
    ""         "$<TARGET_PROPERTY:iface3,NUMBER_MAX_PROP3>"
    ""         "$<TARGET_PROPERTY:iface3,NUMBER_MIN_PROP5>"

    "ON"       "$<TARGET_PROPERTY:static1,BOOL_PROP5>"
    "prop4"    "$<TARGET_PROPERTY:static1,STRING_PROP4>"
    "6"        "$<TARGET_PROPERTY:static1,NUMBER_MIN_PROP6>"
    "4"        "$<TARGET_PROPERTY:static1,NUMBER_MAX_PROP4>"

    "ON"       "$<TARGET_PROPERTY:object1,BOOL_PROP5>"
    "prop4"    "$<TARGET_PROPERTY:object1,STRING_PROP4>"
    "7"        "$<TARGET_PROPERTY:object1,NUMBER_MIN_PROP6>"
    "1"        "$<TARGET_PROPERTY:object1,NUMBER_MAX_PROP4>"

    "ON"       "$<TARGET_PROPERTY:iface3,BOOL_PROP5>"
    "prop4"    "$<TARGET_PROPERTY:iface3,STRING_PROP4>"
    "7"        "$<TARGET_PROPERTY:iface3,NUMBER_MIN_PROP6>"
    "1"        "$<TARGET_PROPERTY:iface3,NUMBER_MAX_PROP4>"
  )
