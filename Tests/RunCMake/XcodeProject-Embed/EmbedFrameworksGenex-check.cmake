# The generator expression must resolve to the "embedded" framework target so
# that an "Embed Frameworks" copy-files phase is created. Were the expression
# left unevaluated, target lookup would fail and no such phase would exist.
execute_process(
  COMMAND grep -c "Embed Frameworks"
                  ${RunCMake_TEST_BINARY_DIR}/${test}.xcodeproj/project.pbxproj
  OUTPUT_VARIABLE actualCount
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT actualCount MATCHES "^[0-9]+$" OR actualCount EQUAL 0)
  set(RunCMake_TEST_FAILED
    "no Embed Frameworks phase in project; generator expression was not honored")
endif()
