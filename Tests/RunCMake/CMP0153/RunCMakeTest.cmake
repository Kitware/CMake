include(RunCMake)

function(run_cmp0153 name)
  run_cmake_command(${name} ${CMAKE_COMMAND} -P "${RunCMake_SOURCE_DIR}/${name}.cmake")
endfunction()

run_cmp0153(CMP0153-WARN)
run_cmp0153(CMP0153-OLD)
run_cmp0153(CMP0153-NEW)
