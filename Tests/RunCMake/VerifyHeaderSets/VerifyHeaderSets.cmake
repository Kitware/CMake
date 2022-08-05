enable_language(C CXX)

add_compile_definitions(TEST_ADD_COMPILE_DEFINITIONS)

set_property(SOURCE a.h PROPERTY LANGUAGE C)
set_property(SOURCE dir/c.h PROPERTY LANGUAGE C)
set_property(SOURCE dir/cxx.h PROPERTY LANGUAGE CXX)

add_library(static STATIC lib.c)
target_sources(static INTERFACE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

add_library(shared SHARED lib.c)
target_sources(shared INTERFACE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

add_library(object OBJECT lib.c)
target_sources(object INTERFACE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

add_library(interface INTERFACE)
target_sources(interface INTERFACE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

add_executable(exe main.c)
target_sources(exe INTERFACE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

add_executable(export_exe main.c)
set_property(TARGET export_exe PROPERTY ENABLE_EXPORTS TRUE)
target_sources(export_exe INTERFACE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

add_library(none STATIC lib.c)

add_library(property_off STATIC lib.c)
target_sources(property_off INTERFACE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)
set_property(TARGET property_off PROPERTY VERIFY_INTERFACE_HEADER_SETS OFF)

add_library(private STATIC lib.c)
target_sources(private PRIVATE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

add_library(a_h STATIC lib.c)
target_compile_definitions(a_h INTERFACE TEST_A_H)
target_sources(a_h INTERFACE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

add_library(dir_c_h STATIC lib.c)
target_compile_definitions(dir_c_h INTERFACE TEST_DIR_C_H)
target_sources(dir_c_h INTERFACE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

add_library(dir_cxx_h STATIC lib.c)
target_compile_definitions(dir_cxx_h INTERFACE TEST_DIR_CXX_H)
target_sources(dir_cxx_h INTERFACE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

set_property(SOURCE debug.h PROPERTY LANGUAGE C)
set_property(SOURCE release.h PROPERTY LANGUAGE C)

if(NOT CMAKE_GENERATOR STREQUAL "Xcode")
  add_library(config STATIC lib.c)
  target_sources(config INTERFACE FILE_SET HEADERS FILES $<IF:$<CONFIG:Debug>,debug.h,release.h>)
endif()

add_library(lang_test_c STATIC lib.c)
target_sources(lang_test_c INTERFACE FILE_SET HEADERS FILES lang_test.h)

add_library(lang_test_cxx STATIC lib.c lib.cxx)
target_compile_definitions(lang_test_cxx INTERFACE EXPECT_CXX)
target_sources(lang_test_cxx INTERFACE FILE_SET HEADERS FILES lang_test.h)

add_library(interface_lang_test_cxx INTERFACE)
target_compile_definitions(interface_lang_test_cxx INTERFACE EXPECT_CXX)
target_sources(interface_lang_test_cxx INTERFACE FILE_SET HEADERS FILES lang_test.h)

set_property(SOURCE error.h PROPERTY LANGUAGE C)

add_library(list STATIC lib.c)
target_sources(list INTERFACE
  FILE_SET a TYPE HEADERS FILES a.h
  FILE_SET c TYPE HEADERS FILES dir/c.h
  FILE_SET error TYPE HEADERS FILES error.h
  )
set_property(TARGET list PROPERTY INTERFACE_HEADER_SETS_TO_VERIFY "a;c")
