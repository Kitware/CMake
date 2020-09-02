include(${RunCMake_TEST_BINARY_DIR}/target_files.cmake)
check_files(${RunCMake_TEST_BINARY_DIR}/out
  ${StaticTop_TARGET_FILE}
  ${StaticPreBuild_TARGET_FILE}
  ${StaticPreLink_TARGET_FILE}
  ${StaticPostBuild_TARGET_FILE}
  ${StaticCc_TARGET_FILE}
  )
