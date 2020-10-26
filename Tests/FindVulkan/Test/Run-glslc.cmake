cmake_minimum_required(VERSION 3.12)

function(run_glslc exe exe_display)
  execute_process(COMMAND ${exe} --help
    OUTPUT_VARIABLE output
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE result
    )

  if(NOT result EQUAL 0)
    message(SEND_ERROR "Result of ${exe_display} --help is ${result}, should be 0")
  endif()

  if(NOT output MATCHES "^glslc - Compile shaders into SPIR-V")
    message(SEND_ERROR "Output of ${exe_display} --help is \"${output}\", should begin with \"glslc - Compile shaders into SPIR-V\"")
  endif()
endfunction()

run_glslc("${VULKAN_GLSLC_EXECUTABLE}" "\${VULKAN_GLSLC_EXECUTABLE}")
run_glslc("${VULKAN_GLSLC_EXECUTABLE_TARGET}" "Vulkan::glslc")
