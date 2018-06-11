if(RunCMake_SUBTEST_SUFFIX MATCHES "^(none|good(_multi)?|invalid_good)")
  check_ext_json("${src_dir}/tests/EXT/expected-json-1.0.txt" "${FOUND_FILE_1}")
endif()
