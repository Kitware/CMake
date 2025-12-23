enable_language(C CXX)

add_compile_definitions(TEST_ADD_COMPILE_DEFINITIONS)

set_property(SOURCE a.h PROPERTY LANGUAGE C)
set_property(SOURCE dir/c.h PROPERTY LANGUAGE C)
set_property(SOURCE dir/cxx.h PROPERTY LANGUAGE CXX)

set(CMAKE_VERIFY_INTERFACE_HEADER_SETS ON)

add_executable(exe main.c)
target_sources(exe INTERFACE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)

add_executable(export_exe main.c)
set_property(TARGET export_exe PROPERTY ENABLE_EXPORTS TRUE)
target_sources(export_exe INTERFACE FILE_SET HEADERS FILES a.h dir/c.h dir/cxx.h)
