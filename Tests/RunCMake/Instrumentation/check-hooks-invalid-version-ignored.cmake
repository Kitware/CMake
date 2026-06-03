if (EXISTS ${v1}/postCMakeBuild.hook)
  string(APPEND RunCMake_TEST_FAILED
    "Invalid-version query should have been ignored, but postCMakeBuild hook ran\n")
endif()
