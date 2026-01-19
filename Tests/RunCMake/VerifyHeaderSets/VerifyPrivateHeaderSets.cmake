cmake_policy(SET CMP0209 NEW)

enable_language(C CXX)

add_compile_definitions(TEST_ADD_COMPILE_DEFINITIONS)

set_property(SOURCE a.h PROPERTY LANGUAGE C)
set_property(SOURCE dir/c.h PROPERTY LANGUAGE C)
set_property(SOURCE dir/cxx.h PROPERTY LANGUAGE CXX)

add_library(static STATIC lib.c)
target_sources(static PRIVATE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

add_library(shared SHARED lib.c)
target_sources(shared PRIVATE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

add_library(module MODULE lib.c)
target_sources(module PRIVATE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

add_library(object OBJECT lib.c)
target_sources(object PRIVATE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

add_library(interface INTERFACE)
target_sources(interface INTERFACE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

# Though a bit strange, INTERFACE libraries can have PRIVATE file sets.
# A more likely scenario would be generated headers which might be PUBLIC.
# Don't make the name of this target any longer or else it triggers a crash in the OpenWatcom compiler.
# That crash only occurs when debug information is enabled, which is the case when this test runs.
add_library(iface_private INTERFACE)
target_sources(iface_private PRIVATE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

add_executable(exe main.c)
target_sources(exe PRIVATE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

add_library(none STATIC lib.c)

add_library(property_off STATIC lib.c)
target_sources(property_off PRIVATE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)
set_property(TARGET property_off PROPERTY VERIFY_PRIVATE_HEADER_SETS OFF)

add_library(a_h STATIC lib.c)
target_compile_definitions(a_h PRIVATE TEST_A_H)
target_sources(a_h PRIVATE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

add_library(dir_c_h STATIC lib.c)
target_compile_definitions(dir_c_h PRIVATE TEST_DIR_C_H)
target_sources(dir_c_h PRIVATE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

add_library(dir_cxx_h STATIC lib.c)
target_compile_definitions(dir_cxx_h PRIVATE TEST_DIR_CXX_H)
target_sources(dir_cxx_h PRIVATE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

set_property(SOURCE debug.h PROPERTY LANGUAGE C)
set_property(SOURCE release.h PROPERTY LANGUAGE C)

if(NOT CMAKE_GENERATOR STREQUAL "Xcode")
  add_library(config STATIC lib.c)
  target_sources(config PRIVATE FILE_SET HEADERS FILES $<IF:$<CONFIG:Debug>,debug.h,release.h>)
endif()

add_library(lang_test_c STATIC lib.c)
target_sources(lang_test_c PRIVATE FILE_SET HEADERS FILES lang_test.h)

add_library(lang_test_cxx STATIC lib.c lib.cxx)
target_compile_definitions(lang_test_cxx PRIVATE EXPECT_CXX)
target_sources(lang_test_cxx PRIVATE FILE_SET HEADERS FILES lang_test.h)

# Don't make the name of this target any longer or else it triggers a crash in the OpenWatcom compiler.
# That crash only occurs when debug information is enabled, which is the case when this test runs.
add_library(iface_lang_cxx INTERFACE)
target_compile_definitions(iface_lang_cxx INTERFACE EXPECT_CXX)
target_sources(iface_lang_cxx INTERFACE FILE_SET HEADERS FILES lang_test.h)

set_property(SOURCE error.h PROPERTY LANGUAGE C)

add_library(list STATIC lib.c)
target_sources(list PRIVATE
  FILE_SET a TYPE HEADERS FILES a.h
  FILE_SET c TYPE HEADERS FILES dir/c.h
  FILE_SET error TYPE HEADERS FILES error.h
)
set_property(TARGET list PROPERTY PRIVATE_HEADER_SETS_TO_VERIFY "a;c")

add_library(skip_linting STATIC lib.c)
target_sources(skip_linting PRIVATE FILE_SET HEADERS FILES lang_test.h skip_linting.h)
set_property(SOURCE skip_linting.h PROPERTY LANGUAGE C)
set_property(SOURCE skip_linting.h PROPERTY SKIP_LINTING TRUE)
