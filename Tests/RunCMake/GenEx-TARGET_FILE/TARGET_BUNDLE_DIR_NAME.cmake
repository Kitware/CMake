enable_language(C)

set(GENERATE_CONTENT [[
macro (check_value test_msg value expected)
  if (NOT "${value}" STREQUAL "${expected}")
    string (APPEND RunCMake_TEST_FAILED "${test_msg}: actual result:\n [${value}]\nbut expected:\n [${expected}]\n")
  endif()
endmacro()
]])

add_library(test-lib MODULE empty.c)
set_target_properties(test-lib PROPERTIES BUNDLE TRUE)

add_library(test-fw empty.c)
set_target_properties(test-fw PROPERTIES FRAMEWORK TRUE)

add_executable(test-app MACOSX_BUNDLE empty.c)

add_executable(test-app-custom MACOSX_BUNDLE empty.c)
set_target_properties(test-app-custom PROPERTIES BUNDLE_EXTENSION custom)

string(APPEND GENERATE_CONTENT [[
check_value("TARGET_BUNDLE_DIR_NAME library" "$<TARGET_BUNDLE_DIR_NAME:test-lib>" "test-lib.bundle")
check_value("TARGET_BUNDLE_DIR_NAME framework" "$<TARGET_BUNDLE_DIR_NAME:test-fw>" "test-fw.framework")
check_value("TARGET_BUNDLE_DIR_NAME app" "$<TARGET_BUNDLE_DIR_NAME:test-app>" "test-app.app")
check_value("TARGET_BUNDLE_DIR_NAME custom" "$<TARGET_BUNDLE_DIR_NAME:test-app-custom>" "test-app-custom.custom")
]])

file(
  GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/TARGET_BUNDLE_DIR_NAME-generated.cmake"
  CONTENT "${GENERATE_CONTENT}"
)
