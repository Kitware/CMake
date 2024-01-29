enable_language(C)
set(CMAKE_CROSSCOMPILING 1)

# Executable: Return error code different from 0
add_executable(generated_exe_emulator_expected simple_src_exiterror.c)
get_property(emulator TARGET generated_exe_emulator_expected PROPERTY CROSSCOMPILING_EMULATOR)
set_property(TARGET generated_exe_emulator_expected PROPERTY CROSSCOMPILING_EMULATOR "$<1:${emulator}>")

# Executable: Return error code equal to 0
add_executable(generated_exe_emulator_unexpected emulator_unexpected.c)
# Place the executable in a predictable location.
set_property(TARGET generated_exe_emulator_unexpected PROPERTY RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_CURRENT_BINARY_DIR}>)

# Executable: Imported version of above.  Fake the imported target to use the above.
add_executable(generated_exe_emulator_unexpected_imported IMPORTED)
set_property(TARGET generated_exe_emulator_unexpected_imported PROPERTY IMPORTED_LOCATION
  "${CMAKE_CURRENT_BINARY_DIR}/generated_exe_emulator_unexpected${CMAKE_EXECUTABLE_SUFFIX}")
add_dependencies(generated_exe_emulator_unexpected_imported generated_exe_emulator_unexpected)

# DoesNotUseEmulator
add_custom_target(generate_output1 ALL
  ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/output1)

# DoesNotUseEmulator: The command will fail if emulator is prepended
add_custom_target(generate_output2 ALL
  ${CMAKE_COMMAND} -E echo "$<TARGET_FILE:generated_exe_emulator_unexpected>"
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/output2
  DEPENDS generated_exe_emulator_unexpected)

# DoesNotUseEmulator: The command will fail if emulator is prepended
add_custom_target(generate_output3 ALL
  $<TARGET_FILE:generated_exe_emulator_unexpected>
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/output3
  DEPENDS generated_exe_emulator_unexpected)

# DoesNotUseEmulator: The command will fail if emulator is prepended
add_custom_target(generate_outputImp ALL
  COMMAND generated_exe_emulator_unexpected_imported
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/outputImp
  )

# UsesEmulator: The command only succeeds if the emulator is prepended
#               to the command.
add_custom_target(generate_output4 ALL
  generated_exe_emulator_expected
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/output4
  DEPENDS generated_exe_emulator_expected)

# Use locally-built emulator.
add_executable(local_emulator ../pseudo_emulator.c)
add_executable(use_emulator_local simple_src_exiterror.c)
set_property(TARGET use_emulator_local PROPERTY CROSSCOMPILING_EMULATOR "$<TARGET_FILE:local_emulator>")
add_dependencies(use_emulator_local local_emulator)
add_custom_target(generate_output5 ALL
  use_emulator_local
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/output5
  DEPENDS use_emulator_local)
