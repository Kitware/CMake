# Give Git access to the real home directory to get user's settings.
if(DEFINED ENV{CTEST_REAL_HOME})
  set(ENV{HOME} "$ENV{CTEST_REAL_HOME}")
endif()

file(GLOB known_files
  "${CMake_SOURCE_DIR}/Tests/Java/hs_err_pid*.log"
  "${CMake_SOURCE_DIR}/Tests/JavaExportImport/InstallExport/hs_err_pid*.log"
  "${CMake_SOURCE_DIR}/Tests/JavaNativeHeaders/hs_err_pid*.log"
  )
if(known_files)
  file(REMOVE ${known_files})
endif()

execute_process(
  COMMAND "${GIT_EXECUTABLE}" status
  WORKING_DIRECTORY "${CMake_SOURCE_DIR}"
  OUTPUT_VARIABLE output
  ERROR_VARIABLE output
  RESULT_VARIABLE result
  )
string(REPLACE "\n" "\n  " output "  ${output}")
if(NOT result EQUAL 0)
  message(FATAL_ERROR "'git status' failed (${result}):\n${output}")
endif()

if(output MATCHES "\n[ \t#]*(Changes |new file:|modified:|Untracked )")
  message(FATAL_ERROR "The source tree is not clean.  'git status' reports:\n${output}")
endif()

message(STATUS "The source tree is clean.")
