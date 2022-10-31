enable_language(C)

if(CMAKE_C_COMPILE_OPTIONS_USE_PCH)
  add_definitions(-DHAVE_PCH_SUPPORT)
endif()

add_library(empty empty.c)
target_precompile_headers(empty PRIVATE
  <stdio.h>
  <string.h>
)
target_include_directories(empty PUBLIC include)

add_library(foo foo.c)
target_include_directories(foo PUBLIC include)
target_precompile_headers(foo REUSE_FROM empty)

# Visual Studio 2017 and greater
if (NOT (CMAKE_C_COMPILER_ID STREQUAL "MSVC" AND CMAKE_C_COMPILER_VERSION VERSION_LESS_EQUAL 19.10))
  set_target_properties(foo PROPERTIES PREFIX "lib" IMPORT_PREFIX "lib")
endif()

add_executable(foobar foobar.c)
target_link_libraries(foobar foo )
set_target_properties(foobar PROPERTIES PRECOMPILE_HEADERS_REUSE_FROM empty)

enable_testing()
add_test(NAME foobar COMMAND foobar)
