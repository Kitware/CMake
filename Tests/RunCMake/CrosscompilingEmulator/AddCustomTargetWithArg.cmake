set(CMAKE_CROSSCOMPILING 1)

# Executable: Return error code different from 0
add_executable(generated_exe_emulator_expected simple_src_exiterror.cxx)

add_custom_target(generate_output ALL
  generated_exe_emulator_expected
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/output
  DEPENDS generated_exe_emulator_expected)
