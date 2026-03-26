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

if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "_valgrind")
  list(APPEND test_exclusions
    # Tests that timeout under valgrind.
    "^RunCMake.NinjaMultiConfig$"
    "^RunCMake.Autogen_Qt6_1$"
    "^RunCMake.GoogleTest$"
    "^RunCMake.CXXModules$"
    "^RunCMake.CXXModulesCompile$"
    "^RunCMake.CommandLine$"

    # Too spurious under Valgrind.
    "^RunCMake.testUVProcessChain$"
    )
endif()

if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "^macos_x86_64_")
  list(APPEND test_exclusions
    # FIXME(#27376): CMakeGUI's simpleConfigure:fail case hangs.
    "^CMakeGUI$"
    )
endif()

if ("$ENV{CMAKE_CONFIGURATION}" MATCHES [[windows_orangec7\.0\.7]])
  list(APPEND test_exclusions
    # FIXME(OrangeC#1136): OrangeC 7 no longer fails when linking a missing library
    "^RunCMake.CheckModules$"
    )
endif()

if ("$ENV{CMAKE_CONFIGURATION}" MATCHES "windows_clang_fastbuild")
  list(APPEND test_exclusions
    # FIXME(#27697): These fail with clang-cl and FASTBuild.
    "^BuildDepends$"
    "^ExportImport$"
    "^Module.CheckIPOSupported-C$"
    "^RunCMake.PrecompileHeaders$"
    "^RunCMake.PrecompileHeaders-Reuse$"
    )
endif()

string(REPLACE ";" "|" test_exclusions "${test_exclusions}")
if (test_exclusions)
  set(test_exclusions "(${test_exclusions})")
endif ()
