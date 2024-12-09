set(in_app1 "${RunCMake_TEST_BINARY_DIR}/Debug/app1.app/Contents/Frameworks/TestLib.framework/Headers")
if(NOT EXISTS "${in_app1}")
  string(APPEND RunCMake_TEST_FAILED "TestLib was embedded without Headers in:\n  ${in_app1}\n")
endif()

set(in_app2 "${RunCMake_TEST_BINARY_DIR}/Debug/app2.app/Contents/Frameworks/TestLib.framework/Headers")
if(EXISTS "${in_app2}")
  string(APPEND RunCMake_TEST_FAILED "TestLib was embedded with Headers in:\n  ${in_app2}\n")
endif()
