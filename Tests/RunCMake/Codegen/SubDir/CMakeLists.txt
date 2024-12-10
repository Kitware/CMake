add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/generated.h
  COMMAND
    ${CMAKE_COMMAND} -E
        copy ${CMAKE_CURRENT_SOURCE_DIR}/../generated.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/generated.h
  CODEGEN
)

add_library(errorlib_subdir
  # If this library is built error.c will cause the build to fail
  ../error.c
  ${CMAKE_CURRENT_BINARY_DIR}/generated.h
)
