
cmake_minimum_required(VERSION 3.16...3.17)

enable_language(C)

add_library (func SHARED func.c)

set (binary_dir "${CMAKE_BINARY_DIR}")
get_property (is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (is_multi_config)
  string (APPEND binary_dir "/Release")
endif()


add_library(import-local SHARED IMPORTED)
set_property(TARGET import-local PROPERTY IMPORTED_LOCATION "${binary_dir}/${CMAKE_STATIC_LIBRARY_PREFIX}func${CMAKE_SHARED_LIBRARY_SUFFIX}")
set_property(TARGET import-local PROPERTY IMPORTED_IMPLIB "${binary_dir}/${CMAKE_STATIC_LIBRARY_PREFIX}func${CMAKE_IMPORT_LIBRARY_SUFFIX}")
add_library(alias::local ALIAS import-local)

add_library (lib-local SHARED lib.c)
target_link_libraries (lib-local PRIVATE alias::local)

add_executable (main-local main.c)
target_link_libraries (main-local PRIVATE alias::local)


add_library(import-global SHARED IMPORTED GLOBAL)
set_property(TARGET import-global PROPERTY IMPORTED_LOCATION "${binary_dir}/${CMAKE_STATIC_LIBRARY_PREFIX}func${CMAKE_SHARED_LIBRARY_SUFFIX}")
set_property(TARGET import-global PROPERTY IMPORTED_IMPLIB "${binary_dir}/${CMAKE_STATIC_LIBRARY_PREFIX}func${CMAKE_IMPORT_LIBRARY_SUFFIX}")
add_library(alias::global ALIAS import-global)

add_library (lib-global SHARED lib.c)
target_link_libraries (lib-global PRIVATE alias::global)

add_executable (main-global main.c)
target_link_libraries (main-global PRIVATE alias::global)


add_subdirectory(sub_dir)
