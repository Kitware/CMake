cmake_minimum_required(VERSION 3.7)
project(mocDepends)

if (QT_TEST_VERSION STREQUAL 4)
  find_package(Qt4 REQUIRED)
  set(QT_CORE_TARGET Qt4::QtCore)
else()
  if (NOT QT_TEST_VERSION STREQUAL 5)
    message(SEND_ERROR "Invalid Qt version specified.")
  endif()

  find_package(Qt5Core REQUIRED)
  set(QT_CORE_TARGET Qt5::Core)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# -- Test 1 using generated header
# This tests the dependency of AUTOMOC of mocDepends1 to the generated object.hpp
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/object.hpp
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/invalid.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/object.hpp
                   COMMAND ${CMAKE_COMMAND} -E sleep 3
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/object.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/object.hpp
                   )

add_executable(mocDepends1 test1.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/object.hpp
)
target_link_libraries(mocDepends1 ${QT_CORE_TARGET})
set_target_properties(mocDepends1 PROPERTIES AUTOMOC TRUE)
set_property(TARGET mocDepends1 PROPERTY __UNDOCUMENTED_AUTOGEN_GENERATED_FILES 1)

# -- Test 2 using generated library
# This tests the dependency of AUTOMOC of mocDepends2 to the
# generated simpleLib.hpp which belongs to a linked library of mocDepends2
add_custom_command(OUTPUT simpleLib.hpp simpleLib.cpp
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/invalid.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/simpleLib.hpp
                   COMMAND ${CMAKE_COMMAND} -E sleep 3
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/simpleLib.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/simpleLib.hpp
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/simpleLib.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/simpleLib.cpp
                   )
add_library(SimpleLib STATIC simpleLib.hpp simpleLib.cpp)

add_executable(mocDepends2 test2.cpp )
target_link_libraries(mocDepends2 SimpleLib ${QT_CORE_TARGET})
set_target_properties(mocDepends2 PROPERTIES AUTOMOC TRUE)
set_property(TARGET mocDepends2 PROPERTY __UNDOCUMENTED_AUTOGEN_GENERATED_FILES 1)
