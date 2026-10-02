if(WIN32)
  check_installed([[^bin;bin/(lib)?runtime_dependency_support\.dll$]])
else()
  check_installed([[^lib;lib/libruntime_dependency_support\.(so|dylib)$]])
endif()

foreach(target IN ITEMS runtime_dependency_shared runtime_dependency_module)
  if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/${RunCMake_TEST_CONFIG}/lib${target}.so" AND
     NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/${RunCMake_TEST_CONFIG}/lib${target}.dylib" AND
     NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/${RunCMake_TEST_CONFIG}/lib${target}.dll" AND
     NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/${RunCMake_TEST_CONFIG}/${target}.dll")
    set(RunCMake_TEST_FAILED "Preparation did not build ${target} for scanning.")
    return()
  endif()
endforeach()
execute_process(COMMAND "${RunCMake_TEST_BINARY_DIR}/runtime-dependency-imported${CMAKE_HOST_EXECUTABLE_SUFFIX}"
  RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  set(RunCMake_TEST_FAILED "Prepared imported runtime dependency root failed: ${result}")
endif()
