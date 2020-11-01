file(MAKE_DIRECTORY "${bin_dir}")
include(ProcessorCount)
ProcessorCount(nproc)
if(generator MATCHES "Ninja")
  set(ninja_arg --generator=Ninja)
endif()
if(NOT nproc EQUAL 0)
  set(parallel_arg --parallel=${nproc})
endif()
message(STATUS "running bootstrap: ${bootstrap} ${ninja_arg} ${parallel_arg}")
execute_process(
  COMMAND ${bootstrap} ${ninja_arg} ${parallel_arg}
  WORKING_DIRECTORY "${bin_dir}"
  RESULT_VARIABLE result
  )
if(result)
  message(FATAL_ERROR "bootstrap failed: ${result}")
endif()
