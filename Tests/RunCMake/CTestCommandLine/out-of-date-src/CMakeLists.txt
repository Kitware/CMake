cmake_minimum_required(VERSION 4.4)
project(OutOfDateDeps C)

enable_testing()

add_executable(app "${CMAKE_CURRENT_SOURCE_DIR}/main.c")
add_library(staticlib STATIC "${CMAKE_CURRENT_SOURCE_DIR}/lib.c")
add_library(sharedlib SHARED "${CMAKE_CURRENT_SOURCE_DIR}/lib.c")
add_library(modulelib MODULE "${CMAKE_CURRENT_SOURCE_DIR}/lib.c")
add_library(objlib OBJECT "${CMAKE_CURRENT_SOURCE_DIR}/lib.c")

set(file_gen "${CMAKE_CURRENT_BINARY_DIR}/file_gen")
add_custom_target(custom COMMAND "${CMAKE_COMMAND}" -E touch ${file_gen})

add_test(NAME exec-test COMMAND app)
add_test(NAME static-test COMMAND "${CMAKE_COMMAND}" -E echo $<TARGET_FILE:staticlib>)
add_test(NAME shared-test COMMAND "${CMAKE_COMMAND}" -E true BUILD_DEPENDS sharedlib)
add_test(NAME module-test COMMAND "${CMAKE_COMMAND}" -E true BUILD_DEPENDS modulelib)
if (CMAKE_GENERATOR STREQUAL "Xcode")
  # Can't get the outputs of the object library, depend on the generated file
  #   instead for parity with other environments.
  add_test(NAME object-test COMMAND "${CMAKE_COMMAND}" -E true BUILD_DEPENDS ${file_gen})
else()
  add_test(NAME object-test COMMAND "${CMAKE_COMMAND}" -E true BUILD_DEPENDS objlib)
endif()
add_test(NAME custom-test COMMAND "${CMAKE_COMMAND}" -E true BUILD_DEPENDS custom)
add_test(NAME nodeps-test COMMAND "${CMAKE_COMMAND}" -E true)
add_test(NAME file-test COMMAND "${CMAKE_COMMAND}" -E true BUILD_DEPENDS ${file_gen})
