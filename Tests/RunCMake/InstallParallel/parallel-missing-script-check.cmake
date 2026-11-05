if (EXISTS "${RunCMake_TEST_BINARY_DIR}/install")
  set(RunCMake_TEST_FAILED
    "files were installed despite a missing install script")
endif ()
