file(READ "${RunCMake_TEST_BINARY_DIR}/GLOBAL_PROPERTY-generated.txt" content)

if(NOT content MATCHES "ENABLED_LANGUAGES=C;CXX" OR NOT content MATCHES "DEBUG_CONFIGURATIONS=Debug;Foo")
  set(RunCMake_TEST_FAILED "actual content:\n [[${content}]]\nbut expected:\n [[ENABLED_LANGUAGES=C;CXX\nDEBUG_CONFIGURATIONS=Debug;Foo]]")
endif()
