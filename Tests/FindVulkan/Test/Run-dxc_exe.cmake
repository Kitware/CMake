cmake_minimum_required(VERSION 3.12)

function(run_dxc_exe exe exe_display)
  execute_process(COMMAND ${exe} --help
    OUTPUT_VARIABLE output
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE result
    )

  if(NOT result EQUAL 0)
    message(SEND_ERROR "Result of ${exe_display} --help is ${result}, should be 0")
  endif()

  if(NOT output MATCHES "^OVERVIEW: HLSL Compiler for ")
    message(SEND_ERROR "Output of ${exe_display} --help is \"${output}\", should begin with \"OVERVIEW: HLSL Compiler for \"")
  endif()
endfunction()

run_dxc_exe("${VULKAN_DXC_EXECUTABLE}" "\${VULKAN_DXC_EXECUTABLE}")
run_dxc_exe("${VULKAN_DXC_EXECUTABLE_TARGET}" "Vulkan::dxc_exe")
