# cmake_print_properties() must reproduce its historical output byte-for-byte.

set(expected "-- \n Properties for TARGET foo:")
string(APPEND expected "\n   foo.MY_PROP = \"val\"")
string(APPEND expected "\n   foo.NOT_SET = <NOTFOUND>")

string(FIND "${actual_stdout}" "${expected}" idx)
if(idx EQUAL -1)
  set(RunCMake_TEST_FAILED
    "cmake_print_properties() output is not byte-for-byte backward compatible.\n"
    "Expected to find:\n[${expected}]\n"
    "in actual stdout:\n[${actual_stdout}]")
endif()
