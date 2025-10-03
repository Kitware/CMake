add_library(TestSymbolicInterfaceLib INTERFACE SYMBOLIC)

get_target_property(IS_SYMBOLIC TestSymbolicInterfaceLib "SYMBOLIC")

if("${IS_SYMBOLIC}" STREQUAL "FALSE")
  string(APPEND RunCMake_TEST_FAILED "Target: \"TestSymbolicInterfaceLib\" does not have the SYMBOLIC property")
  set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
endif()
