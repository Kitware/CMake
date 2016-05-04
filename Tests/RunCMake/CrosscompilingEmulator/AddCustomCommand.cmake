set(CMAKE_CROSSCOMPILING 1)

# Executable: Return error code different from 0
add_executable(generated_exe_emulator_expected simple_src_exiterror.cxx)

# Executable: Return error code equal to 0
add_executable(generated_exe_emulator_unexpected simple_src_exitsuccess.cxx)

# DoesNotUseEmulator
add_custom_command(OUTPUT output1
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/output1)

# DoesNotUseEmulator: The command will fail if emulator is prepended
add_custom_command(OUTPUT output2
  COMMAND ${CMAKE_COMMAND} -E echo "$<TARGET_FILE:generated_exe_emulator_unexpected>"
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/output2
  DEPENDS generated_exe_emulator_unexpected)

# DoesNotUseEmulator: The command will fail if emulator is prepended
add_custom_command(OUTPUT output3
  COMMAND $<TARGET_FILE:generated_exe_emulator_unexpected>
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/output3
  DEPENDS generated_exe_emulator_unexpected)

# UsesEmulator: The command only succeeds if the emulator is prepended
#               to the command.
add_custom_command(OUTPUT output4
  COMMAND generated_exe_emulator_expected
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/output4
  DEPENDS generated_exe_emulator_expected)

add_custom_target(ensure_build ALL
  SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/output1
    ${CMAKE_CURRENT_BINARY_DIR}/output2
    ${CMAKE_CURRENT_BINARY_DIR}/output3
    ${CMAKE_CURRENT_BINARY_DIR}/output4
)
