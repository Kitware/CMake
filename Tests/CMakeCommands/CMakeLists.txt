macro(add_CMakeCommands_test test)
  add_test(CMakeCommands.${test} ${CMAKE_CMAKE_COMMAND}
    -DCMake_SOURCE_DIR=${CMake_SOURCE_DIR} # TODO: Remove
    -Ddir=${CMAKE_CURRENT_BINARY_DIR}/${test}
    -Dgen=${CMAKE_TEST_GENERATOR}
    -P "${CMAKE_CURRENT_SOURCE_DIR}/${test}/test.cmake"
    )
endmacro()

add_CMakeCommands_test(build_command)
