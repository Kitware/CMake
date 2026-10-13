add_generated_file(file_set_header file_set.h)
add_generated_file(public_header public.h)

add_library(subdirectory_headers STATIC EXCLUDE_FROM_ALL
  "${generated_target_source}" "${public_header}")
target_sources(subdirectory_headers PUBLIC
  FILE_SET HEADERS
  BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
  FILES "${file_set_header}"
)
set_property(TARGET subdirectory_headers
  PROPERTY PUBLIC_HEADER "${public_header}")
