project(use_prefix)

set(use_prefix_lib_SRCS
  useprefixclass.cpp
)

add_library(use_prefix_lib SHARED useprefixclass.cpp)

generate_export_header(use_prefix_lib
  PREFIX_NAME MYPREFIX_
)

add_executable(use_prefix main.cpp)

target_link_libraries(use_prefix use_prefix_lib)