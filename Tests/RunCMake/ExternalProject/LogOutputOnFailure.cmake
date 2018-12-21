include(ExternalProject)

set(dummyOutput [[
This is some dummy output with some long lines to ensure formatting is preserved
    Including lines with leading spaces

And also blank lines
]])

ExternalProject_Add(FailsWithOutput
  SOURCE_DIR            ${CMAKE_CURRENT_LIST_DIR}
  CONFIGURE_COMMAND     ""
  BUILD_COMMAND         ${CMAKE_COMMAND} -E echo ${dummyOutput}
        COMMAND         ${CMAKE_COMMAND} -E env  # missing command, forces fail
  TEST_COMMAND          ""
  INSTALL_COMMAND       ""
  LOG_BUILD             YES
  LOG_OUTPUT_ON_FAILURE YES
  USES_TERMINAL_BUILD   YES
)
