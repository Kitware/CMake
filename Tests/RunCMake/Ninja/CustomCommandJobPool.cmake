set_property(GLOBAL PROPERTY JOB_POOLS custom_command_pool=2 custom_target_pool=2)
add_custom_command(
  OUTPUT hello.copy.c
  COMMAND "${CMAKE_COMMAND}" -E copy
          "${CMAKE_CURRENT_SOURCE_DIR}/hello.c"
          hello.copy.c
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  JOB_POOL "custom_command_pool"
  )
add_custom_target(copy ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/hello.copy.c")

add_custom_target(
  hello.echo
  COMMAND echo
  JOB_POOL "custom_target_pool"
)

include(CheckNoPrefixSubDir.cmake)
