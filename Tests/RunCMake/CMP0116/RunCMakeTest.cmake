include(RunCMake)

function(run_cmp0116 status warn)
  if(warn)
    set(name CMP0116-${status}-WARN)
  else()
    set(name CMP0116-${status}-NOWARN)
  endif()
  set(RunCMake_TEST_OPTIONS
    -DCMAKE_POLICY_WARNING_CMP0116:BOOL=${warn}
    )
  if(NOT status STREQUAL "WARN")
    list(APPEND RunCMake_TEST_OPTIONS
      -DCMAKE_POLICY_DEFAULT_CMP0116:STRING=${status}
      )
  endif()

  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${name}-build)
  run_cmake(${name})
  unset(RunCMake_TEST_OPTIONS)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake-check-file check.cmake)

  file(TOUCH "${RunCMake_TEST_BINARY_DIR}/topdep.txt")
  file(TOUCH "${RunCMake_TEST_BINARY_DIR}/Subdirectory/subdep.txt")
  set(cmp0116_step 1)
  run_cmake_command(${name}-build1 ${CMAKE_COMMAND} --build . --config Debug)
  file(REMOVE "${RunCMake_TEST_BINARY_DIR}/topstamp.txt")
  file(REMOVE "${RunCMake_TEST_BINARY_DIR}/Subdirectory/substamp.txt")
  execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1.25)

  file(TOUCH "${RunCMake_TEST_BINARY_DIR}/topdep.txt")
  file(TOUCH "${RunCMake_TEST_BINARY_DIR}/Subdirectory/subdep.txt")
  set(cmp0116_step 2)
  run_cmake_command(${name}-build2 ${CMAKE_COMMAND} --build . --config Debug)
  file(REMOVE "${RunCMake_TEST_BINARY_DIR}/topstamp.txt")
  file(REMOVE "${RunCMake_TEST_BINARY_DIR}/Subdirectory/substamp.txt")
  execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1.25)

  set(cmp0116_step 3)
  run_cmake_command(${name}-build3 ${CMAKE_COMMAND} --build . --config Debug)
endfunction()

run_cmp0116(WARN OFF)
run_cmp0116(OLD OFF)
run_cmp0116(NEW OFF)
run_cmp0116(WARN ON)
run_cmp0116(OLD ON)
run_cmp0116(NEW ON)

set(RunCMake_TEST_OPTIONS -DCMAKE_POLICY_WARNING_CMP0116:BOOL=TRUE)
run_cmake(CMP0116-Mixed)
