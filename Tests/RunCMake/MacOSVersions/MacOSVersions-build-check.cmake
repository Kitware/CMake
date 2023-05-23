set(cfg_dir)
if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(cfg_dir /Debug)
endif()

set(lib "${RunCMake_TEST_BINARY_DIR}${cfg_dir}/libfoo.1.0.dylib")
if(NOT EXISTS "${lib}")
  set(RunCMake_TEST_FAILED "Library file is missing:\n  ${lib}")
  return()
endif()

execute_process(COMMAND otool -l "${lib}" OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE res)
if(NOT res EQUAL 0)
  string(REPLACE "\n" "\n  " err "  ${err}")
  set(RunCMake_TEST_FAILED "Running 'otool -l' on file:\n  ${lib}\nfailed:\n${err}")
  return()
endif()

foreach(ver
    [[current version 3\.2\.1]]
    [[compatibility version 2\.1\.0]]
    )
  if(NOT "${out}" MATCHES "( |\n)${ver}( |\n)")
    string(CONCAT RunCMake_TEST_FAILED "Library file:\n  ${lib}\n" "does not contain '${ver}'")
    return()
  endif()
endforeach()
