cmake_instrumentation(
  API_VERSION 1
  DATA_VERSION 1
  HOOKS preBuild postBuild
  CALLBACK "\"${CMAKE_COMMAND}\" -P \"${CMAKE_SOURCE_DIR}/../hook.cmake\" 0"
)
