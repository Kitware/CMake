include(GeneratedCommon.cmake)

add_generated_file(generated_header generated.h)
add_library(generated_headers STATIC EXCLUDE_FROM_ALL
  "${generated_target_source}")
target_sources(generated_headers PUBLIC
  FILE_SET HEADERS
  BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
  FILES "${generated_header}"
)
install(TARGETS generated_headers
  ARCHIVE DESTINATION lib
    COMPONENT Archive
    EXCLUDE_FROM_ALL
  FILE_SET HEADERS DESTINATION include
    COMPONENT Headers
)

# A file set without generated files does not need its target built.
add_library(plain_headers STATIC EXCLUDE_FROM_ALL neverbuild.c)
target_sources(plain_headers PUBLIC
  FILE_SET HEADERS
  FILES plain.h
)
install(TARGETS plain_headers
  ARCHIVE DESTINATION lib
    COMPONENT Archive
    EXCLUDE_FROM_ALL
  FILE_SET HEADERS DESTINATION include
    COMPONENT Headers
)

add_library(unit_tests STATIC neverbuild.c)
