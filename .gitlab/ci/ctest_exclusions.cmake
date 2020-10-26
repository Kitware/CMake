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

if (CMAKE_HOST_WIN32)
  list(APPEND test_exclusions
    # This test often fails with an undiagnosed subtle race due to the test
    # re-using the same objects for many files.  Some copy operations fail
    # to open their input with ERROR_SHARING_VIOLATION.
    "^Module.ExternalData$"
    )
endif()

string(REPLACE ";" "|" test_exclusions "${test_exclusions}")
if (test_exclusions)
  set(test_exclusions "(${test_exclusions})")
endif ()
