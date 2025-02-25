  # Query 0
  cmake_instrumentation(
    API_VERSION 1
    DATA_VERSION 1
  )
  # Query 1
  cmake_instrumentation(
    API_VERSION 1
    DATA_VERSION 1
    HOOKS postGenerate
    CALLBACK ${CMAKE_COMMAND} -E echo callback1
  )
  # Query 2
  cmake_instrumentation(
    API_VERSION 1
    DATA_VERSION 1
    HOOKS postCMakeBuild
    QUERIES staticSystemInformation dynamicSystemInformation
    CALLBACK ${CMAKE_COMMAND} -E echo callback2
    CALLBACK ${CMAKE_COMMAND} -E echo callback3
  )
