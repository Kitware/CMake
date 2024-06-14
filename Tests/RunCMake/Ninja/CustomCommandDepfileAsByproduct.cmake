add_custom_command(
  OUTPUT hello.copy.c
  BYPRODUCTS "test.d"
  COMMAND "${CMAKE_COMMAND}" -E copy
          "${CMAKE_CURRENT_SOURCE_DIR}/hello.c"
          hello.copy.c
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  DEPFILE "test.d"
  )

add_custom_command(
  OUTPUT hello.copy2.c
  BYPRODUCTS "test_$<CONFIG>.d"
  COMMAND "${CMAKE_COMMAND}" -E copy
          "${CMAKE_CURRENT_SOURCE_DIR}/hello.c"
          hello.copy2.c
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  DEPFILE "test_$<CONFIG>.d"
  )

add_custom_target(copy ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/hello.copy.c"
  "${CMAKE_CURRENT_BINARY_DIR}/hello.copy2.c")

include(CheckNoPrefixSubDir.cmake)
