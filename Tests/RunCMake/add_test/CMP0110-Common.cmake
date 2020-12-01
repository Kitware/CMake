include(CTest)
add_test(
  NAME "${TEST_NAME}"
  COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_LIST_DIR}/CMP0110-Test.cmake"
)
set_property(
  TEST "${TEST_NAME}"
  PROPERTY ENVIRONMENT CMAKE_add_test_ENVVAR=1
)
