cmake_host_system_information(RESULT msbuild QUERY VS_MSBUILD_COMMAND)
if(NOT EXISTS "${msbuild}")
  message(FATAL_ERROR "VS_MSBUILD_COMMAND returned path that does not exist:\n ${msbuild}")
endif()
