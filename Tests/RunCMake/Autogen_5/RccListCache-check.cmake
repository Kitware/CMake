file(GLOB_RECURSE info_files
  "${RunCMake_TEST_BINARY_DIR}/AutoRcc_res_*_Info.json")
if(NOT info_files)
  set(RunCMake_TEST_FAILED "AutoRcc info file not found.")
  return()
endif()
list(GET info_files 0 info_file)
file(READ "${info_file}" info)

foreach(name IN LISTS expect_inputs)
  if(NOT info MATCHES "/${name}\"")
    string(APPEND RunCMake_TEST_FAILED
      "Resource \"${name}\" is missing from the inputs in\n  ${info_file}\n")
  endif()
endforeach()
foreach(name IN LISTS expect_no_inputs)
  if(info MATCHES "/${name}\"")
    string(APPEND RunCMake_TEST_FAILED
      "Resource \"${name}\" is unexpectedly in the inputs in\n  ${info_file}\n")
  endif()
endforeach()

file(GLOB_RECURSE cache_files
  "${RunCMake_TEST_BINARY_DIR}/AutoRcc_res_*_List*.txt")
if(expect_cache AND NOT cache_files)
  string(APPEND RunCMake_TEST_FAILED "The rcc listing was not cached.\n")
elseif(NOT expect_cache AND cache_files)
  string(APPEND RunCMake_TEST_FAILED
    "The rcc listing was unexpectedly cached in\n  ${cache_files}\n")
endif()
