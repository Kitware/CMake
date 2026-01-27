set(log "${RunCMake_BINARY_DIR}/SourceFileJobPool-build/build.ninja")
file(READ "${log}" build_file)
if(NOT "${build_file}" MATCHES "pool = source_file_compile_pool")
  string(CONCAT RunCMake_TEST_FAILED "Log file:\n ${log}\n" "does not have expected line: pool = source_file_compile_pool")
endif()
if(NOT "${build_file}" MATCHES "pool = target_link_pool")
  string(CONCAT RunCMake_TEST_FAILED "Log file:\n ${log}\n" "does not have expected line: pool = target_link_pool")
endif()
# Even though `target_compile_pool` was defined as the target's compile jobs pool, since the only sourcefile
# of the target overrides it with `source_file_compile_pool` pool, the target compile job pool should not
# exist in the generated Ninja file.
if("${build_file}" MATCHES "pool = target_compile_pool")
  string(CONCAT RunCMake_TEST_FAILED "Log file:\n ${log}\n" "have unexpected line: pool = target_compile_pool")
endif()
