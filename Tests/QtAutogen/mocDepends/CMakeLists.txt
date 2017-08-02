cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0071 NEW)
project(mocDepends CXX)

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

# -- Test 1: Depend on generated header
# The ORIGIN_autogen target must depend on the same *GENERATED* source files as
# the ORIGIN target. This is a requirement to ensure that all files for the
# ORIGIN target are generated before the ORIGIN_autogen target is built.
#
# This tests the dependency of the mocDepends1_autogen target of mocDepends1
# to the source file test1_object.hpp, which is *GENERATED* by a custom command.
# If mocDepends1_autogen gets built *before* or in *parallel* to the
# custom command, the build will fail. That's because test1_object.hpp,
# which is required by mocDepends1_autogen, is only valid after the
# custom command has been completed.
#
# The sleep seconds artificially increase the build time of the custom command
# to simulate a slow file generation process that takes longer to run than
# the build of the mocDepends1_autogen target.
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test1_object.hpp
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/invalid.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/test1_object.hpp
  COMMAND ${CMAKE_COMMAND} -E sleep 3
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/object.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/test1_object.hpp)

add_executable(mocDepends1 test1.cpp ${CMAKE_CURRENT_BINARY_DIR}/test1_object.hpp)
target_link_libraries(mocDepends1 ${QT_CORE_TARGET})
set_target_properties(mocDepends1 PROPERTIES AUTOMOC TRUE)

# -- Test 2: Depend on header generating target
# The ORIGIN_autogen target must depend on the same user defined targets
# as the ORIGIN target. This is a requirement to ensure that all files for the
# ORIGIN target are generated before the ORIGIN_autogen target is built.
#
# This tests the dependency of the mocDepends2_autogen target of mocDepends2
# to the utility target mocDepends2Object. If mocDepends2_autogen gets built
# *before* or in *parallel* to mocDepends2Object, the build will fail. That's
# because test2_object.hpp, which is required by mocDepends2_autogen,
# is only valid after the mocDepends2Object build has been completed.
#
# The sleep seconds artificially increase the build time of mocDepends2Object
# to simulate a slow utility target build that takes longer to run than
# the build of the mocDepends2_autogen target.
add_custom_target(mocDepends2Object
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/test2_object.hpp
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/invalid.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/test2_object.hpp
  COMMAND ${CMAKE_COMMAND} -E sleep 3
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/object.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/test2_object.hpp)

add_executable(mocDepends2 test2.cpp)
target_link_libraries(mocDepends2 ${QT_CORE_TARGET})
set_target_properties(mocDepends2 PROPERTIES AUTOMOC TRUE)
add_dependencies(mocDepends2 mocDepends2Object)

# -- Test 3: Depend on generated linked library
# The ORIGIN_autogen target must depend on the same linked libraries
# as the ORIGIN target. This is a requirement to ensure that all files for the
# ORIGIN target are generated before the ORIGIN_autogen target is built.
#
# This tests the dependency of the mocDepends3_autogen target of mocDepends3
# to the user generated library SimpleLib, which mocDepends3 links to.
# If mocDepends3_autogen gets built *before* or in *parallel* to SimpleLib,
# the build will fail. That's because simpleLib.hpp, which is required by
# mocDepends3_autogen, is only valid after the SimpleLib build has been
# completed.
#
# The sleep seconds artificially increase the build time of SimpleLib
# to simulate a slow utility library build that takes longer to run than
# the build of the mocDepends3_autogen target.
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/simpleLib.hpp ${CMAKE_CURRENT_BINARY_DIR}/simpleLib.cpp
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/invalid.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/simpleLib.hpp
  COMMAND ${CMAKE_COMMAND} -E sleep 3
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/simpleLib.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/simpleLib.hpp
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/simpleLib.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/simpleLib.cpp)
add_library(SimpleLib STATIC ${CMAKE_CURRENT_BINARY_DIR}/simpleLib.hpp ${CMAKE_CURRENT_BINARY_DIR}/simpleLib.cpp)
target_link_libraries(SimpleLib ${QT_CORE_TARGET})

add_executable(mocDepends3 test3.cpp)
target_link_libraries(mocDepends3 SimpleLib ${QT_CORE_TARGET})
set_target_properties(mocDepends3 PROPERTIES AUTOMOC TRUE)
