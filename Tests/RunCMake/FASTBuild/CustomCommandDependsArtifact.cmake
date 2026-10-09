# Custom commands depend on the artifact of a target that is defined after
# them, by full path and by a path relative to the binary directory.  The
# target must be built before the commands run.
set(produced "produced${CMAKE_EXECUTABLE_SUFFIX}")

add_custom_command(OUTPUT absolute.txt
  COMMAND ${CMAKE_COMMAND} -E touch absolute.txt
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${produced}"
)
add_custom_command(OUTPUT relative.txt
  COMMAND ${CMAKE_COMMAND} -E touch relative.txt
  DEPENDS "${produced}"
)
add_custom_target(consumer DEPENDS absolute.txt relative.txt)

add_executable(producer main.cpp)
set_target_properties(producer PROPERTIES
  # N.B. "produced" here differs from "producer" so that the OLD behavior of
  # CMP0212 is not tripped, and the test exercises full file path resolution.
  OUTPUT_NAME produced
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
