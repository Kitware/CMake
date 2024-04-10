set(CMAKE_EXPERIMENTAL_WINDOWS_KERNEL_MODE_DRIVER
  "5c2d848d-4efa-4529-a768-efd57171bf68")

cmake_language(GET_EXPERIMENTAL_FEATURE_ENABLED
  "WindowsKernelModeDriver"
  feature_present)

if (NOT feature_present STREQUAL "TRUE")
  message(FATAL_ERROR
    "Expected the `WindowsKernelModeDriver` feature to be enabled.")
endif ()
