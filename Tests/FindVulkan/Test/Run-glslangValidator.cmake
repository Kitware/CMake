cmake_minimum_required(VERSION 3.12)

function(run_glslangValidator exe exe_display)
  execute_process(COMMAND ${exe} --help
    OUTPUT_VARIABLE output
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE result
    )

  if(NOT result EQUAL 1)
    message(SEND_ERROR "Result of ${exe_display} --help is ${result}, should be 1")
  endif()

  # NOTE: Newer version prefer just "glslang" since it's no longer really just a validator.
  # This approach is still compatible with older version that output glslangValidator
  if(NOT output MATCHES "^Usage: glslang")
    message(SEND_ERROR "Output of ${exe_display} --help is \"${output}\", should begin with \"Usage: glslang\"")
  endif()
endfunction()

run_glslangValidator("${VULKAN_GLSLANG_VALIDATOR_EXECUTABLE}" "\${VULKAN_GLSLANG_VALIDATOR_EXECUTABLE}")
run_glslangValidator("${VULKAN_GLSLANG_VALIDATOR_EXECUTABLE_TARGET}" "Vulkan::glslangValidator")
