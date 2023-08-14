include(RunCMake)

run_cmake(ParseImplicitLinkInfo)

run_cmake(Inspect)
set(info "${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")
include("${info}")

if(INFO_CMAKE_C_IMPLICIT_LINK_DIRECTORIES MATCHES ";")
  run_cmake_with_options(ExcludeDirs "-Dinfo=${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")
endif()
