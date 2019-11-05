verify_ctest_resources()

set(expected_contents [[
begin Test1
alloc transmogrifiers calvin 2
begin Test2
alloc transmogrifiers hobbes 2
dealloc transmogrifiers calvin 2
end Test1
dealloc transmogrifiers hobbes 2
end Test2
]])
file(READ "${RunCMake_TEST_BINARY_DIR}/ctresalloc.log" actual_contents)
if(NOT actual_contents STREQUAL expected_contents)
  string(APPEND RunCMake_TEST_FAILED "ctresalloc.log contents did not match expected\n")
endif()
