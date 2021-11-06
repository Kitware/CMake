if(CMake_TEST_CUDA STREQUAL "NVIDIA")
  set(expected "-Xlinker=OPT1 -Xlinker=OPT2 -Xlinker=OPT3 -Xlinker=OPT4 -Xlinker=OPT5")
elseif(CMake_TEST_CUDA STREQUAL "Clang")
  set(expected "-Wl,OPT1 -Xlinker OPT2 -Xlinker OPT3 -Xlinker OPT4")
endif()

if(NOT actual_stdout MATCHES "${expected}")
    set(RunCMake_TEST_FAILED "Not found expected '${expected}'")
endif()
