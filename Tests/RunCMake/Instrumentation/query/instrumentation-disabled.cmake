cmake_instrumentation(
  API_VERSION 1
  DATA_VERSION 1
  OPTIONS staticSystemInformation dynamicSystemInformation captureOutput
          processMetrics trace cdashSubmit cdashVerbose
  HOOKS postGenerate preBuild postBuild preCMakeBuild postCMakeBuild
        postCTest postCMakeInstall
  CALLBACK ${CMAKE_COMMAND} -E touch "${CMAKE_BINARY_DIR}/instrumentation-hook"
)
