set(test_exclusions
  # This test hits global resources and can be handled by nightly testing.
  # https://gitlab.kitware.com/cmake/cmake/-/merge_requests/4769
  "^BundleGeneratorTest$"
)

if (CTEST_CMAKE_GENERATOR MATCHES "Visual Studio")
  list(APPEND test_exclusions
    # This test takes around 5 minutes with Visual Studio.
    # https://gitlab.kitware.com/cmake/cmake/-/issues/20733
    "^ExternalProjectUpdate$"
    # This test is a dependency of the above and is only required for it.
    "^ExternalProjectUpdateSetup$")
endif ()

if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "_asan")
  list(APPEND test_exclusions
    CTestTest2 # crashes on purpose
    BootstrapTest # no need to cover this for asan
    )
endif()

if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "_jom")
  list(APPEND test_exclusions
    # JOM often fails with "Couldn't change working directory to ...".
    "^ExternalProject$"
    )
endif()

if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "nvhpc_")
  list(APPEND test_exclusions
    # FIXME(#24187): This test fails with NVHPC as the CUDA host compiler.
    "^CudaOnly.SeparateCompilationPTX$"
    )
endif()

if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "fedora39_makefiles")
  list(APPEND test_exclusions
    # FIXME(#25421): CPack/RPM needs an update for Fedora 39.
    "^CPackComponentsForAll-RPM-default$"
    "^CPackComponentsForAll-RPM-AllInOne$"
    "^CPackComponentsForAll-RPM-OnePackPerGroup$"
    )
endif()

string(REPLACE ";" "|" test_exclusions "${test_exclusions}")
if (test_exclusions)
  set(test_exclusions "(${test_exclusions})")
endif ()
