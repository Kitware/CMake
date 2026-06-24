if(NOT DEFINED RunCMake_TEST_BINARY_DIR)
  message(FATAL_ERROR "RunCMake_TEST_BINARY_DIR not set")
endif()

if(NOT DEFINED expect_present)
  message(FATAL_ERROR "expect_present not set")
endif()

file(GLOB build_files LIST_DIRECTORIES false
  "${RunCMake_TEST_BINARY_DIR}/*.ninja"
  "${RunCMake_TEST_BINARY_DIR}/*.ninja.in"
  "${RunCMake_TEST_BINARY_DIR}/fbuild.bff"
  "${RunCMake_TEST_BINARY_DIR}/Makefile"
  "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/Makefile2")

set(found FALSE)
foreach(build_file IN LISTS build_files)
  file(READ "${build_file}" content)
  if(content MATCHES "test_prep.all|test_prep.TargetBuildTest")
    set(found TRUE)
    break()
  endif()
endforeach()

if(expect_present AND NOT found)
  message(FATAL_ERROR "Expected test_prep targets to be present in build files.")
endif()

if(NOT expect_present AND found)
  message(FATAL_ERROR "Expected test_prep targets to be absent from build files.")
endif()
