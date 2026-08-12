enable_language(CXX)

find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core)

set(CMAKE_AUTOMOC ON)

add_library(simple_lib STATIC simple_lib.cpp)

# A plain AUTOMOC target gets the autogen custom command attached to itself.
add_library(plain_lib STATIC app_qt.cpp)
target_link_libraries(plain_lib PRIVATE simple_lib Qt${with_qt_version}::Core)

# A target that depends on a GENERATED file keeps the separate autogen targets.
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated_dep.h
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/generated_dep.h)
add_library(gen_dep_lib STATIC app_qt.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/generated_dep.h)
target_link_libraries(gen_dep_lib PRIVATE Qt${with_qt_version}::Core)
target_include_directories(gen_dep_lib PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
set_property(TARGET gen_dep_lib PROPERTY
  AUTOGEN_TARGET_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/autogen_dep.txt)
