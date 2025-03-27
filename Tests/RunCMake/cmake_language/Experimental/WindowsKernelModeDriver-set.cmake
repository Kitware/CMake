set(CMAKE_EXPERIMENTAL_WINDOWS_KERNEL_MODE_DRIVER
  "fac18f65-504e-4dbb-b068-f356bb1f2ddb")

cmake_language(GET_EXPERIMENTAL_FEATURE_ENABLED
  "WindowsKernelModeDriver"
  feature_present)

if (NOT feature_present STREQUAL "TRUE")
  message(FATAL_ERROR
    "Expected the `WindowsKernelModeDriver` feature to be enabled.")
endif ()
