
set(schema "${RunCMake_TEST_BINARY_DIR}/XcodeSchemaGeneration.xcodeproj/xcshareddata/xcschemes/foo.xcscheme")

if(NOT EXISTS "${schema}")
  set(RunCMake_TEST_FAILED "Generated schema ${schema} does not exist.")
  return()
endif()

execute_process(COMMAND
  /usr/bin/xmllint --xpath "//Scheme/ProfileAction/BuildableProductRunnable" ${schema}
  OUTPUT_VARIABLE stdout
  ERROR_VARIABLE stderr
  RESULT_VARIABLE exit_code
  ERROR_STRIP_TRAILING_WHITESPACE
)
if(exit_code)
  set(RunCMake_TEST_FAILED "Failed to find BuildableProductRunnable for profile action: ${stderr}")
  return()
endif()
