set(CMAKE_EXPERIMENTAL_WINDOWS_KERNEL_MODE_DRIVER
  "7f524e81-99c7-48f3-a35d-278bae54282c")

cmake_language(GET_EXPERIMENTAL_FEATURE_ENABLED
  "WindowsKernelModeDriver"
  feature_present)

if (NOT feature_present STREQUAL "TRUE")
  message(FATAL_ERROR
    "Expected the `WindowsKernelModeDriver` feature to be enabled.")
endif ()
