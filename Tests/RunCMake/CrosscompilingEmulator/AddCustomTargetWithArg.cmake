enable_language(C)
set(CMAKE_CROSSCOMPILING 1)

# Executable: Return error code different from 0
add_executable(generated_exe_emulator_expected simple_src_exiterror.c)
get_property(emulator TARGET generated_exe_emulator_expected PROPERTY CROSSCOMPILING_EMULATOR)
set_property(TARGET generated_exe_emulator_expected PROPERTY CROSSCOMPILING_EMULATOR "$<1:${emulator}>")

add_custom_target(generate_output ALL
  generated_exe_emulator_expected
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/output
  DEPENDS generated_exe_emulator_expected)
