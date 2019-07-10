include(RunCMake)
include(RunCTest)

###############################################################################
# Test cthwalloc itself - we want to make sure it's not just rubber-stamping
# the test results
###############################################################################

function(cthwalloc_verify_log expected_contents)
  if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/cthwalloc.log")
    string(APPEND RunCMake_TEST_FAILED "Log file was not written\n")
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
    return()
  endif()
  file(READ "${RunCMake_TEST_BINARY_DIR}/cthwalloc.log" actual_contents)
  if(NOT actual_contents STREQUAL expected_contents)
    string(APPEND RunCMake_TEST_FAILED "Actual log did not match expected log\n")
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
  endif()
endfunction()

function(run_cthwalloc_write_proc name proc)
  file(REMOVE "${RunCMake_BINARY_DIR}/${name}-build/cthwalloc.log")
  run_cthwalloc_write_proc_nodel("${name}" "${proc}" "${ARGN}")
endfunction()

function(run_cthwalloc_write_proc_nodel name proc)
  string(REPLACE ";" "\\;" proc "${proc}")
  run_cmake_command(${name} "${CMAKE_COMMAND}" -E env "${ARGN}" "${CTHWALLOC_COMMAND}" write "${RunCMake_BINARY_DIR}/${name}-build/cthwalloc.log" "${name}" 0 "${proc}")
endfunction()

function(run_cthwalloc_write_noproc name)
  run_cmake_command(${name} "${CMAKE_COMMAND}" -E env "${ARGN}" "${CTHWALLOC_COMMAND}" write "${RunCMake_BINARY_DIR}/${name}-build/cthwalloc.log" "${name}" 0)
endfunction()

function(run_cthwalloc_verify name tests)
  string(REPLACE ";" "\\;" tests "${tests}")
  run_cmake_command(${name} "${CTHWALLOC_COMMAND}" verify "${RunCMake_SOURCE_DIR}/${name}.log" "${CMAKE_CURRENT_LIST_DIR}/hwspec.json" "${tests}")
endfunction()

unset(ENV{CTEST_PROCESS_COUNT})
set(RunCMake_TEST_NO_CLEAN 1)
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/cthwalloc-write-proc-good1-build")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/cthwalloc-write-proc-good1-build")
file(WRITE "${RunCMake_BINARY_DIR}/cthwalloc-write-proc-good1-build/cthwalloc.log"
[[begin test1
alloc widgets 0 1
dealloc widgets 0 1
end test1
]])
run_cthwalloc_write_proc_nodel(cthwalloc-write-proc-good1 "1,widgets:2,transmogrifiers:1;2,widgets:1,widgets:2"
  CTEST_PROCESS_COUNT=3
  CTEST_PROCESS_0=widgets,transmogrifiers
  CTEST_PROCESS_0_WIDGETS=id:0,slots:2
  CTEST_PROCESS_0_TRANSMOGRIFIERS=id:calvin,slots:1
  CTEST_PROCESS_1=widgets
  "CTEST_PROCESS_1_WIDGETS=id:0,slots:1\\;id:2,slots:2"
  CTEST_PROCESS_2=widgets
  "CTEST_PROCESS_2_WIDGETS=id:0,slots:1\\;id:2,slots:2"
  )
set(RunCMake_TEST_NO_CLEAN 0)
run_cthwalloc_write_proc(cthwalloc-write-proc-good2 "widgets:8"
  CTEST_PROCESS_COUNT=1
  CTEST_PROCESS_0=widgets
  CTEST_PROCESS_0_WIDGETS=id:3,slots:8
  )
run_cthwalloc_write_proc(cthwalloc-write-proc-nocount "widgets:8")
run_cthwalloc_write_proc(cthwalloc-write-proc-badcount "widgets:8"
  CTEST_PROCESS_COUNT=2
  )
run_cthwalloc_write_proc(cthwalloc-write-proc-nores "widgets:8"
  CTEST_PROCESS_COUNT=1
  )
run_cthwalloc_write_proc(cthwalloc-write-proc-badres "widgets:8"
  CTEST_PROCESS_COUNT=1
  CTEST_PROCESS_0=widgets,transmogrifiers
  )
run_cthwalloc_write_proc(cthwalloc-write-proc-nowidgets "widgets:8"
  CTEST_PROCESS_COUNT=1
  CTEST_PROCESS_0=widgets
  )
run_cthwalloc_write_proc(cthwalloc-write-proc-badwidgets1 "widgets:8"
  CTEST_PROCESS_COUNT=1
  CTEST_PROCESS_0=widgets
  CTEST_PROCESS_0_WIDGETS=
  )
run_cthwalloc_write_proc(cthwalloc-write-proc-badwidgets2 "widgets:8"
  CTEST_PROCESS_COUNT=1
  CTEST_PROCESS_0=widgets
  "CTEST_PROCESS_0_WIDGETS=id:3,slots:8\\;id:0,slots:1"
  )
run_cthwalloc_write_proc(cthwalloc-write-proc-badwidgets3 "widgets:8"
  CTEST_PROCESS_COUNT=1
  CTEST_PROCESS_0=widgets
  CTEST_PROCESS_0_WIDGETS=id:3,slots:7
  )
run_cthwalloc_write_proc(cthwalloc-write-proc-badwidgets4 "widgets:8"
  CTEST_PROCESS_COUNT=1
  CTEST_PROCESS_0=widgets
  CTEST_PROCESS_0_WIDGETS=invalid
  )
run_cthwalloc_write_proc(cthwalloc-write-proc-badwidgets5 "widgets:2,widgets:2"
  CTEST_PROCESS_COUNT=1
  CTEST_PROCESS_0=widgets
  "CTEST_PROCESS_0_WIDGETS=id:0,slots:2\\;id:0,slots:1"
  )
run_cthwalloc_write_proc(cthwalloc-write-proc-badwidgets6 "widgets:2"
  CTEST_PROCESS_COUNT=1
  CTEST_PROCESS_0=widgets
  "CTEST_PROCESS_0_WIDGETS=id:0,slots:2\\;id:0,slots:1"
  )
run_cthwalloc_write_proc(cthwalloc-write-proc-badwidgets7 "widgets:2,widgets:2"
  CTEST_PROCESS_COUNT=1
  CTEST_PROCESS_0=widgets
  CTEST_PROCESS_0_WIDGETS=id:0,slots:2
  )

run_cthwalloc_write_noproc(cthwalloc-write-noproc-good1)
run_cthwalloc_write_noproc(cthwalloc-write-noproc-count
  CTEST_PROCESS_COUNT=1
  )

run_cthwalloc_verify(cthwalloc-verify-good1 "test1;test2")
run_cthwalloc_verify(cthwalloc-verify-good2 "")
run_cthwalloc_verify(cthwalloc-verify-nolog "")
run_cthwalloc_verify(cthwalloc-verify-nores "")
run_cthwalloc_verify(cthwalloc-verify-noid "")
run_cthwalloc_verify(cthwalloc-verify-notenough "")
run_cthwalloc_verify(cthwalloc-verify-baddealloc "")
run_cthwalloc_verify(cthwalloc-verify-leak "")
run_cthwalloc_verify(cthwalloc-verify-badtest1 "")
run_cthwalloc_verify(cthwalloc-verify-badtest2 "test1")
run_cthwalloc_verify(cthwalloc-verify-badtest3 "test1")
run_cthwalloc_verify(cthwalloc-verify-badtest4 "test1")
run_cthwalloc_verify(cthwalloc-verify-badtest5 "test1")
run_cthwalloc_verify(cthwalloc-verify-nobegin "test1")
run_cthwalloc_verify(cthwalloc-verify-noend "test1")

###############################################################################
# Now test the hardware allocation feature of CTest
###############################################################################

function(run_ctest_hardware name parallel random)
  run_ctest("${name}-ctest-s-hw" "-DCTEST_HARDWARE_ALLOC_ENABLED=1" "-DCTHWALLOC_COMMAND=${CTHWALLOC_COMMAND}" "-DCTEST_PARALLEL=${parallel}" "-DCTEST_RANDOM=${random}")
  run_ctest("${name}-ctest-s-nohw" "-DCTEST_HARDWARE_ALLOC_ENABLED=0" "-DCTHWALLOC_COMMAND=${CTHWALLOC_COMMAND}" "-DCTEST_PARALLEL=${parallel}" "-DCTEST_RANDOM=${random}")
endfunction()

function(verify_ctest_hardware)
  file(READ "${RunCMake_TEST_BINARY_DIR}/hwtests.txt" hwtests)
  execute_process(COMMAND "${CTHWALLOC_COMMAND}" verify "${RunCMake_TEST_BINARY_DIR}/cthwalloc.log" "${CMAKE_CURRENT_LIST_DIR}/hwspec.json" "${hwtests}"
    OUTPUT_VARIABLE output ERROR_QUIET RESULT_VARIABLE result)
  if(result)
    string(APPEND RunCMake_TEST_FAILED "${output}")
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
  endif()
endfunction()

run_ctest_hardware(lotsoftests 10 1)
run_ctest_hardware(checkfree1 2 0)
run_ctest_hardware(checkfree2 1 0)
run_ctest_hardware(notenough1 1 0)
run_ctest_hardware(notenough2 1 0)
run_ctest_hardware(ensure_parallel 2 0)

set(ENV{CTEST_PROCESS_COUNT} 2)
run_ctest_hardware(process_count 1 0)
unset(ENV{CTEST_PROCESS_COUNT})
