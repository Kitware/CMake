# undocumented, do not use outside of CMake
cmake_language(GET_EXPERIMENTAL_FEATURE_ENABLED "WindowsKernelModeDriver" _cmake_windows_kernel_mode_driver_enabled)
if(NOT _cmake_windows_kernel_mode_driver_enabled)
  message(FATAL_ERROR "Windows kernel-mode driver experimental support is not enabled.")
endif()

if(CMAKE_GENERATOR MATCHES "Visual Studio")
  message(FATAL_ERROR "Visual Studio generators do not yet support CMAKE_SYSTEM_NAME=WindowsKernelModeDriver.")
endif()

set(_CMAKE_FEATURE_DETECTION_TARGET_TYPE STATIC_LIBRARY)
