set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
set(CMAKE_TRY_COMPILE_NO_PLATFORM_VARIABLES ON)
enable_language(SYCL)

try_compile(result
  SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src.sycl"
  OUTPUT_VARIABLE output)
if(NOT result)
  message(FATAL_ERROR
    "SYCL try_compile failed with platform variable forwarding disabled:\n"
    "${output}")
endif()
