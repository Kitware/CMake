include(GeneratedCommon.cmake)

# The target and its generated headers are defined in a subdirectory but
# installed from here, so the generated files must be found in the target's
# directory rather than the directory of the install rule.
add_subdirectory(generated)

install(TARGETS subdirectory_headers
  ARCHIVE DESTINATION lib
    COMPONENT Archive
    EXCLUDE_FROM_ALL
  FILE_SET HEADERS DESTINATION include
    COMPONENT Headers
  PUBLIC_HEADER DESTINATION include/public
    COMPONENT Headers
)

add_library(unit_tests STATIC neverbuild.c)
