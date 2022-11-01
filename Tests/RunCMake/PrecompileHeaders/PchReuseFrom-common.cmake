enable_language(C)

if(CMAKE_C_COMPILE_OPTIONS_USE_PCH)
  add_definitions(-DHAVE_PCH_SUPPORT)
endif()

# Add this before the target from which we will reuse the PCH
# to test that generators can handle reversed ordering.
add_library(foo foo.c)
target_include_directories(foo PUBLIC include)

add_library(empty empty.c)
target_precompile_headers(empty PRIVATE
  <stdio.h>
  <string.h>
)
target_include_directories(empty PUBLIC include)

target_precompile_headers(foo REUSE_FROM empty)

# should not cause problems if configured multiple times
target_precompile_headers(foo REUSE_FROM empty)

add_executable(foobar foobar.c)
target_link_libraries(foobar foo )
set_target_properties(foobar PROPERTIES PRECOMPILE_HEADERS_REUSE_FROM foo)

enable_testing()
add_test(NAME foobar COMMAND foobar)
