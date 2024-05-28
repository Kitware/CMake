add_executable(foobar main.c)
add_custom_command(
  TARGET foobar POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E
    copy ${CMAKE_CURRENT_SOURCE_DIR}/generated.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/foobar.txt
)

add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/generated.h
  COMMAND
    ${CMAKE_COMMAND} -E
        copy ${CMAKE_CURRENT_SOURCE_DIR}/generated.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/generated.h
  COMMAND
    # Generate a header file that requires foobar
    foobar
  CODEGEN
)

add_library(errorlib
  # If this library is built error.c will cause the build to fail
  error.c
  ${CMAKE_CURRENT_BINARY_DIR}/generated.h
)
