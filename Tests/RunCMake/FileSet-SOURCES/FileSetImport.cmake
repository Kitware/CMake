enable_language(C)

get_property(multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

function(assert_target_prop_eq tgt prop value)
  unset(actual_value)
  get_property(actual_value TARGET ${tgt} PROPERTY ${prop})
  if(NOT actual_value STREQUAL value)
    message(SEND_ERROR "Expected value of target ${prop}:\n  ${value}\nActual value:\n  ${actual_value}")
  endif()
endfunction()

function(assert_fileset_prop_eq tgt fs prop value)
  unset(actual_value)
  get_property(actual_value FILE_SET ${fs} TARGET ${tgt} PROPERTY ${prop})
  if(NOT actual_value STREQUAL value)
    message(SEND_ERROR "Expected value of file set ${prop}:\n  ${value}\nActual value:\n  ${actual_value}")
  endif()
endfunction()

cmake_path(GET CMAKE_BINARY_DIR PARENT_PATH export_build_dir)
cmake_path(APPEND export_build_dir "FileSetExport-build")

if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" VERSION_EQUAL 4.3
    AND NOT CMAKE_PATCH_VERSION VERSION_LESS 20000000)
  # development version for future 4.4: Force version 4.4
  set(CMAKE_VERSION_BACKUP "${CMAKE_VERSION}")
  set(CMAKE_VERSION 4.4)
endif()
include("${export_build_dir}/export.cmake")
include("${export_build_dir}/install/lib/cmake/export.cmake")
if(CMAKE_VERSION VERSION_GREATER 4.3 AND CMAKE_VERSION VERSION_LESS 4.4
    AND NOT CMake_VERSION_PATCH VERSION_LESS 20000000)
  # development version for future 4.4: Force version 4.4
  set(CMAKE_VERSION "${CMAKE_VERSION_BACKUP}")
endif()


assert_target_prop_eq(export::lib1 SOURCE_SETS "")
assert_target_prop_eq(export::lib1 INTERFACE_SOURCE_SETS "a;SOURCES")
assert_target_prop_eq(export::lib1 SOURCE_SET_a "${CMAKE_CURRENT_SOURCE_DIR}/lib2.c")
assert_target_prop_eq(export::lib1 SOURCE_SET "${CMAKE_CURRENT_SOURCE_DIR}/lib5.c")
assert_target_prop_eq(export::lib1 SOURCE_SET_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/lib5.c")

assert_fileset_prop_eq(export::lib1 a INTERFACE_COMPILE_DEFINITIONS "INTERFACE_LIB1_A")
assert_fileset_prop_eq(export::lib1 a INTERFACE_COMPILE_OPTIONS "-DOPT_INTERFACE_LIB1_A")
assert_fileset_prop_eq(export::lib1 a INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/subdir2")

assert_fileset_prop_eq(export::lib1 SOURCES INTERFACE_COMPILE_DEFINITIONS "INTERFACE_LIB1_SRCS")
assert_fileset_prop_eq(export::lib1 SOURCES INTERFACE_COMPILE_OPTIONS "-DOPT_INTERFACE_LIB1_SRCS")
assert_fileset_prop_eq(export::lib1 SOURCES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/srcs")


assert_target_prop_eq(install::lib1 SOURCE_SETS "")
assert_target_prop_eq(install::lib1 INTERFACE_SOURCE_SETS "a;SOURCES")
assert_target_prop_eq(install::lib1 SOURCE_SET_a "${export_build_dir}/install/sources/lib2.c")
if(multi_config)
  if(CMAKE_GENERATOR MATCHES "Xcode")
    assert_target_prop_eq(install::lib1 SOURCE_SET "${export_build_dir}/install/sources/srcs/lib5.c")
    assert_target_prop_eq(install::lib1 SOURCE_SET_SOURCES "${export_build_dir}/install/sources/srcs/lib5.c")
  else()
    assert_target_prop_eq(install::lib1 SOURCE_SET "$<$<CONFIG:Debug>:${export_build_dir}/install/sources/debug/lib5.c>;$<$<CONFIG:Release>:${export_build_dir}/install/sources/release/lib5.c>")
    assert_target_prop_eq(install::lib1 SOURCE_SET_SOURCES "$<$<CONFIG:Debug>:${export_build_dir}/install/sources/debug/lib5.c>;$<$<CONFIG:Release>:${export_build_dir}/install/sources/release/lib5.c>")
  endif()
else()
  assert_target_prop_eq(install::lib1 SOURCE_SET "${export_build_dir}/install/sources/debug/lib5.c")
  assert_target_prop_eq(install::lib1 SOURCE_SET_SOURCES "${export_build_dir}/install/sources/debug/lib5.c")
endif()

assert_fileset_prop_eq(install::lib1 a INTERFACE_COMPILE_DEFINITIONS "INTERFACE_LIB1_A")
assert_fileset_prop_eq(install::lib1 a INTERFACE_COMPILE_OPTIONS "-DOPT_INTERFACE_LIB1_A")
assert_fileset_prop_eq(install::lib1 a INTERFACE_INCLUDE_DIRECTORIES "${export_build_dir}/install/include/subdir2")

assert_fileset_prop_eq(install::lib1 SOURCES INTERFACE_COMPILE_DEFINITIONS "INTERFACE_LIB1_SRCS")
assert_fileset_prop_eq(install::lib1 SOURCES INTERFACE_COMPILE_OPTIONS "-DOPT_INTERFACE_LIB1_SRCS")
assert_fileset_prop_eq(install::lib1 SOURCES INTERFACE_INCLUDE_DIRECTORIES "${export_build_dir}/install/include/srcs")


add_executable(main_export main.c)
target_link_libraries(main_export PRIVATE export::lib1)
target_compile_definitions(main_export PRIVATE CONSUMER)

add_executable(main_install main.c)
target_link_libraries(main_install PRIVATE install::lib1)
target_compile_definitions(main_install PRIVATE CONSUMER)
