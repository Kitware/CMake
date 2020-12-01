include(${RunCMake_TEST_BINARY_DIR}/target_files.cmake)
check_files(${RunCMake_TEST_BINARY_DIR}/out
  ${FortranTop_TARGET_FILE}
  ${CMiddle_TARGET_FILE}
  ${FortranBottom_TARGET_FILE}
  )
