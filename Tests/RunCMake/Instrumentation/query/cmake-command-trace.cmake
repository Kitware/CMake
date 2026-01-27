file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}/../hook.cmake" hook_path)
cmake_instrumentation(
  API_VERSION 1
  DATA_VERSION 1
  OPTIONS trace
  HOOKS postBuild postCMakeInstall postCTest
  CALLBACK ${CMAKE_COMMAND} -P ${hook_path} 0 1
)
