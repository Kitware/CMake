file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}/../hook.cmake" hook_path)
cmake_instrumentation(
  API_VERSION 1
  DATA_VERSION 1
  HOOKS preBuild postBuild
  CALLBACK ${CMAKE_COMMAND} -P ${hook_path} 0
)
