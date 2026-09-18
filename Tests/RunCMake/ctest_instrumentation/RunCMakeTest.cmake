include(RunCTest)

function(run_InstrumentationInCTestXML CASE_NAME)
  cmake_parse_arguments(ARGS "USE_INSTRUMENTATION_ENV_VARS;USE_VERBOSE_INSTRUMENTATION;USE_INSTRUMENTATION_CMD;USE_LOCAL_INSTRUMENTATION;USE_STALE_CDASH;USE_PROCESS_METRICS" "" "" ${ARGN})
  if(ARGS_USE_VERBOSE_INSTRUMENTATION)
    set(ENV{CTEST_USE_VERBOSE_INSTRUMENTATION} "1")
    set(RunCMake_USE_VERBOSE_INSTRUMENTATION 1)
  else()
    set(ENV{CTEST_USE_VERBOSE_INSTRUMENTATION} "0")
    set(RunCMake_USE_VERBOSE_INSTRUMENTATION 0)
  endif()
  if(ARGS_USE_INSTRUMENTATION_ENV_VARS)
    set(ENV{CTEST_USE_INSTRUMENTATION} "1")
  else()
    set(ENV{CTEST_USE_INSTRUMENTATION} "0")
  endif()
  if (ARGS_USE_INSTRUMENTATION_CMD)
    set(ENV{USE_INSTRUMENTATION_CMD} "1")
    set(RunCMake_USE_VERBOSE_INSTRUMENTATION 1)
  else()
    set(ENV{USE_INSTRUMENTATION_CMD} "0")
    set(RunCMake_USE_VERBOSE_INSTRUMENTATION 0)
  endif()

  if(ARGS_USE_LOCAL_INSTRUMENTATION)
    set(CASE_CMAKELISTS_SUFFIX_CODE [[
cmake_instrumentation(
  API_VERSION 1
  DATA_VERSION 1
)
]])
  endif()

  if(ARGS_USE_STALE_CDASH)
    set(CASE_CTEST_PREFIX_CODE [=[
# Simulate staged data left by an earlier run with cdashSubmit enabled.
foreach(subdir configure build/commands build/targets/old-target)
  file(WRITE "${CTEST_BINARY_DIRECTORY}/.cmake/instrumentation/v1/cdash/${subdir}/old.json"
    [[{"command":"old","role":"custom","dynamicSystemInformation":{"afterHostMemoryUsed":123}}]]
  )
endforeach()
]=])
  endif()

  if(ARGS_USE_PROCESS_METRICS)
    set(ENV{USE_PROCESS_METRICS} "1")
  else()
    set(ENV{USE_PROCESS_METRICS} "0")
  endif()

  configure_file(${RunCMake_SOURCE_DIR}/main.c
                 ${RunCMake_BINARY_DIR}/${CASE_NAME}/main.c COPYONLY)
  run_ctest("${CASE_NAME}")
  unset(RunCMake_USE_LAUNCHERS)
  unset(RunCMake_USE_INSTRUMENTATION)
endfunction()
run_InstrumentationInCTestXML(NoInstrumentationInCTestXML)
run_InstrumentationInCTestXML(InstrumentationInCTestXML
  USE_INSTRUMENTATION_ENV_VARS
)
run_InstrumentationInCTestXML(VerboseInstrumentationInCTestXML
  USE_INSTRUMENTATION_ENV_VARS
  USE_VERBOSE_INSTRUMENTATION
)
run_InstrumentationInCTestXML(InstrumentationInCTestXMLWithCmd
  USE_INSTRUMENTATION_CMD
)
run_InstrumentationInCTestXML(InstrumentationWithoutCDashSubmit
  USE_LOCAL_INSTRUMENTATION
)
run_InstrumentationInCTestXML(InstrumentationWithoutCDashSubmitWithStaleData
  USE_LOCAL_INSTRUMENTATION USE_STALE_CDASH
)
run_InstrumentationInCTestXML(ProcessMetricsInCTestXML
  USE_PROCESS_METRICS
)
