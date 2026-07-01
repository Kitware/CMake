if (EXISTS "${RunCMake_TEST_BINARY_DIR}/install_manifest.txt")
  set(RunCMake_TEST_FAILED
    "install_manifest.txt was written despite a failed parallel install")
endif ()
