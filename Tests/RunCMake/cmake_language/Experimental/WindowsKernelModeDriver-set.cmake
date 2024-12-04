set(CMAKE_EXPERIMENTAL_WINDOWS_KERNEL_MODE_DRIVER
  "9157bf90-2313-44d6-aefa-67cd83c8be7c")

cmake_language(GET_EXPERIMENTAL_FEATURE_ENABLED
  "WindowsKernelModeDriver"
  feature_present)

if (NOT feature_present STREQUAL "TRUE")
  message(FATAL_ERROR
    "Expected the `WindowsKernelModeDriver` feature to be enabled.")
endif ()
