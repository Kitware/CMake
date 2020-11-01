include(RunCMake)
include(RunCTest)

###############################################################################
# Test ctresalloc itself - we want to make sure it's not just rubber-stamping
# the test results
###############################################################################

function(ctresalloc_verify_log expected_contents)
  if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/ctresalloc.log")
    string(APPEND RunCMake_TEST_FAILED "Log file was not written\n")
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
    return()
  endif()
  file(READ "${RunCMake_TEST_BINARY_DIR}/ctresalloc.log" actual_contents)
  if(NOT actual_contents STREQUAL expected_contents)
    string(APPEND RunCMake_TEST_FAILED "Actual log did not match expected log\n")
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
  endif()
endfunction()

function(run_ctresalloc_write_proc name proc)
  file(REMOVE "${RunCMake_BINARY_DIR}/${name}-build/ctresalloc.log")
  run_ctresalloc_write_proc_nodel("${name}" "${proc}" "${ARGN}")
endfunction()

function(run_ctresalloc_write_proc_nodel name proc)
  string(REPLACE ";" "\\;" proc "${proc}")
  run_cmake_command(${name} "${CMAKE_COMMAND}" -E env "${ARGN}" "${CTRESALLOC_COMMAND}" write "${RunCMake_BINARY_DIR}/${name}-build/ctresalloc.log" "${name}" 0 "${proc}")
endfunction()

function(run_ctresalloc_write_noproc name)
  run_cmake_command(${name} "${CMAKE_COMMAND}" -E env "${ARGN}" "${CTRESALLOC_COMMAND}" write "${RunCMake_BINARY_DIR}/${name}-build/ctresalloc.log" "${name}" 0)
endfunction()

function(run_ctresalloc_verify name tests)
  string(REPLACE ";" "\\;" tests "${tests}")
  run_cmake_command(${name} "${CTRESALLOC_COMMAND}" verify "${RunCMake_SOURCE_DIR}/${name}.log" "${CMAKE_CURRENT_LIST_DIR}/resspec.json" "${tests}")
endfunction()

function(read_testing_file filename variable)
  file(READ "${RunCMake_TEST_BINARY_DIR}/Testing/TAG" _tag)
  string(REGEX REPLACE "^([^\n]*)\n.*$" "\\1" _date "${_tag}")
  file(READ "${RunCMake_TEST_BINARY_DIR}/Testing/${_date}/${filename}" _contents)
  set("${variable}" "${_contents}" PARENT_SCOPE)
endfunction()

unset(ENV{CTEST_RESOURCE_GROUP_COUNT})
set(RunCMake_TEST_NO_CLEAN 1)
file(REMOVE_RECURSE "${RunCMake_BINARY_DIR}/ctresalloc-write-proc-good1-build")
file(MAKE_DIRECTORY "${RunCMake_BINARY_DIR}/ctresalloc-write-proc-good1-build")
file(WRITE "${RunCMake_BINARY_DIR}/ctresalloc-write-proc-good1-build/ctresalloc.log"
[[begin test1
alloc widgets 0 1
dealloc widgets 0 1
end test1
]])
run_ctresalloc_write_proc_nodel(ctresalloc-write-proc-good1 "1,widgets:2,transmogrifiers:1;2,widgets:1,widgets:2"
  CTEST_RESOURCE_GROUP_COUNT=3
  CTEST_RESOURCE_GROUP_0=widgets,transmogrifiers
  CTEST_RESOURCE_GROUP_0_WIDGETS=id:0,slots:2
  CTEST_RESOURCE_GROUP_0_TRANSMOGRIFIERS=id:calvin,slots:1
  CTEST_RESOURCE_GROUP_1=widgets
  "CTEST_RESOURCE_GROUP_1_WIDGETS=id:0,slots:1\\;id:2,slots:2"
  CTEST_RESOURCE_GROUP_2=widgets
  "CTEST_RESOURCE_GROUP_2_WIDGETS=id:0,slots:1\\;id:2,slots:2"
  )
set(RunCMake_TEST_NO_CLEAN 0)
run_ctresalloc_write_proc(ctresalloc-write-proc-good2 "widgets:8"
  CTEST_RESOURCE_GROUP_COUNT=1
  CTEST_RESOURCE_GROUP_0=widgets
  CTEST_RESOURCE_GROUP_0_WIDGETS=id:3,slots:8
  )
run_ctresalloc_write_proc(ctresalloc-write-proc-nocount "widgets:8")
run_ctresalloc_write_proc(ctresalloc-write-proc-badcount "widgets:8"
  CTEST_RESOURCE_GROUP_COUNT=2
  )
run_ctresalloc_write_proc(ctresalloc-write-proc-nores "widgets:8"
  CTEST_RESOURCE_GROUP_COUNT=1
  )
run_ctresalloc_write_proc(ctresalloc-write-proc-badres "widgets:8"
  CTEST_RESOURCE_GROUP_COUNT=1
  CTEST_RESOURCE_GROUP_0=widgets,transmogrifiers
  )
run_ctresalloc_write_proc(ctresalloc-write-proc-nowidgets "widgets:8"
  CTEST_RESOURCE_GROUP_COUNT=1
  CTEST_RESOURCE_GROUP_0=widgets
  )
run_ctresalloc_write_proc(ctresalloc-write-proc-badwidgets1 "widgets:8"
  CTEST_RESOURCE_GROUP_COUNT=1
  CTEST_RESOURCE_GROUP_0=widgets
  CTEST_RESOURCE_GROUP_0_WIDGETS=
  )
run_ctresalloc_write_proc(ctresalloc-write-proc-badwidgets2 "widgets:8"
  CTEST_RESOURCE_GROUP_COUNT=1
  CTEST_RESOURCE_GROUP_0=widgets
  "CTEST_RESOURCE_GROUP_0_WIDGETS=id:3,slots:8\\;id:0,slots:1"
  )
run_ctresalloc_write_proc(ctresalloc-write-proc-badwidgets3 "widgets:8"
  CTEST_RESOURCE_GROUP_COUNT=1
  CTEST_RESOURCE_GROUP_0=widgets
  CTEST_RESOURCE_GROUP_0_WIDGETS=id:3,slots:7
  )
run_ctresalloc_write_proc(ctresalloc-write-proc-badwidgets4 "widgets:8"
  CTEST_RESOURCE_GROUP_COUNT=1
  CTEST_RESOURCE_GROUP_0=widgets
  CTEST_RESOURCE_GROUP_0_WIDGETS=invalid
  )
run_ctresalloc_write_proc(ctresalloc-write-proc-badwidgets5 "widgets:2,widgets:2"
  CTEST_RESOURCE_GROUP_COUNT=1
  CTEST_RESOURCE_GROUP_0=widgets
  "CTEST_RESOURCE_GROUP_0_WIDGETS=id:0,slots:2\\;id:0,slots:1"
  )
run_ctresalloc_write_proc(ctresalloc-write-proc-badwidgets6 "widgets:2"
  CTEST_RESOURCE_GROUP_COUNT=1
  CTEST_RESOURCE_GROUP_0=widgets
  "CTEST_RESOURCE_GROUP_0_WIDGETS=id:0,slots:2\\;id:0,slots:1"
  )
run_ctresalloc_write_proc(ctresalloc-write-proc-badwidgets7 "widgets:2,widgets:2"
  CTEST_RESOURCE_GROUP_COUNT=1
  CTEST_RESOURCE_GROUP_0=widgets
  CTEST_RESOURCE_GROUP_0_WIDGETS=id:0,slots:2
  )

run_ctresalloc_write_noproc(ctresalloc-write-noproc-good1)
run_ctresalloc_write_noproc(ctresalloc-write-noproc-count
  CTEST_RESOURCE_GROUP_COUNT=1
  )

run_ctresalloc_verify(ctresalloc-verify-good1 "test1;test2")
run_ctresalloc_verify(ctresalloc-verify-good2 "")
run_ctresalloc_verify(ctresalloc-verify-nolog "")
run_ctresalloc_verify(ctresalloc-verify-nores "")
run_ctresalloc_verify(ctresalloc-verify-noid "")
run_ctresalloc_verify(ctresalloc-verify-notenough "")
run_ctresalloc_verify(ctresalloc-verify-baddealloc "")
run_ctresalloc_verify(ctresalloc-verify-leak "")
run_ctresalloc_verify(ctresalloc-verify-badtest1 "")
run_ctresalloc_verify(ctresalloc-verify-badtest2 "test1")
run_ctresalloc_verify(ctresalloc-verify-badtest3 "test1")
run_ctresalloc_verify(ctresalloc-verify-badtest4 "test1")
run_ctresalloc_verify(ctresalloc-verify-badtest5 "test1")
run_ctresalloc_verify(ctresalloc-verify-nobegin "test1")
run_ctresalloc_verify(ctresalloc-verify-noend "test1")

###############################################################################
# Now test the resource allocation feature of CTest
###############################################################################

function(run_ctest_resource name parallel random extra)
  run_ctest("${name}-ctest-s-res" "-DCTEST_RESOURCE_ALLOC_ENABLED=1" "-DCTEST_RESOURCE_SPEC_SOURCE=ARG" "-DCTRESALLOC_COMMAND=${CTRESALLOC_COMMAND}" "-DCTEST_PARALLEL=${parallel}" "-DCTEST_RANDOM=${random}")
  run_ctest("${name}-ctest-s-nores" "-DCTEST_RESOURCE_ALLOC_ENABLED=0" "-DCTEST_RESOURCE_SPEC_SOURCE=NONE" "-DCTRESALLOC_COMMAND=${CTRESALLOC_COMMAND}" "-DCTEST_PARALLEL=${parallel}" "-DCTEST_RANDOM=${random}")
  if(extra)
    run_ctest("${name}-ctest-s-res-variable" "-DCTEST_RESOURCE_ALLOC_ENABLED=1" "-DCTEST_RESOURCE_SPEC_SOURCE=VARIABLE" "-DCTRESALLOC_COMMAND=${CTRESALLOC_COMMAND}" "-DCTEST_PARALLEL=${parallel}" "-DCTEST_RANDOM=${random}")
    run_ctest("${name}-ctest-s-res-cache" "-DCTEST_RESOURCE_ALLOC_ENABLED=1" "-DCTEST_RESOURCE_SPEC_SOURCE=CACHE" "-DCTRESALLOC_COMMAND=${CTRESALLOC_COMMAND}" "-DCTEST_PARALLEL=${parallel}" "-DCTEST_RANDOM=${random}")
    run_ctest("${name}-ctest-s-res-cmdline" "-DCTEST_RESOURCE_ALLOC_ENABLED=1" "-DCTEST_RESOURCE_SPEC_SOURCE=CMDLINE" "-DCTRESALLOC_COMMAND=${CTRESALLOC_COMMAND}" "-DCTEST_PARALLEL=${parallel}" "-DCTEST_RANDOM=${random}" --resource-spec-file "${RunCMake_SOURCE_DIR}/resspec.json")
  endif()
endfunction()

function(verify_ctest_resources)
  file(READ "${RunCMake_TEST_BINARY_DIR}/restests.txt" restests)
  execute_process(COMMAND "${CTRESALLOC_COMMAND}" verify "${RunCMake_TEST_BINARY_DIR}/ctresalloc.log" "${CMAKE_CURRENT_LIST_DIR}/resspec.json" "${restests}"
    OUTPUT_VARIABLE output ERROR_QUIET RESULT_VARIABLE result)
  if(result)
    string(APPEND RunCMake_TEST_FAILED "${output}")
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
  endif()
endfunction()

run_ctest_resource(lotsoftests 10 1 0)
run_ctest_resource(checkfree1 2 0 1)
run_ctest_resource(checkfree2 1 0 0)
run_ctest_resource(notenough1 1 0 1)
run_ctest_resource(notenough2 1 0 0)
run_ctest_resource(notenough3 1 0 0)
run_ctest_resource(combine 1 0 0)
run_ctest_resource(ensure_parallel 2 0 0)

set(ENV{CTEST_RESOURCE_GROUP_COUNT} 2)
run_ctest_resource(process_count 1 0 0)
unset(ENV{CTEST_RESOURCE_GROUP_COUNT})
