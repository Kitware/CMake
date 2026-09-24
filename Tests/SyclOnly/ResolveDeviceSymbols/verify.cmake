if(ARCHIVER MATCHES "(^|[/\\\\])(llvm-)?lib(\\.exe)?$")
  set(list_options /nologo /list)
else()
  set(list_options t)
endif()
execute_process(COMMAND "${ARCHIVER}" ${list_options} "${LIBRARY}"
  TIMEOUT 30
  RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "${ARCHIVER} failed: ${result}\n${output}\n${error}")
endif()
if(output MATCHES "(device|device_extra|dependency)\\.sycl")
  message(FATAL_ERROR "Dependency host objects were absorbed into the finalized archive")
endif()
