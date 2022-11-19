enable_language(C)

add_library(foo foo.c)
target_include_directories(foo PUBLIC include)
target_precompile_headers(foo PUBLIC
  include/foo.h
  <stdio.h>
  \"string.h\"
)

add_executable(foobar foobar.c)
target_link_libraries(foobar foo)
set_target_properties(foobar PROPERTIES DISABLE_PRECOMPILE_HEADERS ON)
