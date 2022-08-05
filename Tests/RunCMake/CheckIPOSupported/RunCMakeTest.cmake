include(RunCMake)

run_cmake(Inspect)
include("${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")

run_cmake(unparsed-arguments)
run_cmake(user-lang-unknown)
run_cmake(default-lang-none)
run_cmake(not-supported-by-cmake)
run_cmake(not-supported-by-compiler)
run_cmake(save-to-result)
run_cmake(cmp0069-is-old)

if(_CMAKE_C_IPO_SUPPORTED_BY_CMAKE
    AND _CMAKE_C_IPO_MAY_BE_SUPPORTED_BY_COMPILER
    AND NOT RunCMake_GENERATOR MATCHES "^Visual Studio 9 ")
  run_cmake(CMP0138-WARN)
  run_cmake(CMP0138-OLD)
  run_cmake(CMP0138-NEW)
endif()

if(RunCMake_GENERATOR MATCHES "^Visual Studio 9 ")
  run_cmake(not-supported-by-generator)
endif()
