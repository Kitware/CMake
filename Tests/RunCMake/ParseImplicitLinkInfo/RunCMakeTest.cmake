include(RunCMake)

run_cmake(ParseImplicitLinkInfo)

run_cmake(Inspect)
set(info "${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")
include("${info}")

if(CMAKE_HOST_UNIX)
  run_cmake_script(DetermineLinkerId)
endif()

if(INFO_CMAKE_C_IMPLICIT_LINK_DIRECTORIES MATCHES ";")
  run_cmake_with_options(ExcludeDirs "-Dinfo=${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")
endif()
