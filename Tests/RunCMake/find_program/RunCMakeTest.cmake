include(RunCMake)

run_cmake(EnvAndHints)
run_cmake(DirsPerName)
run_cmake(NamesPerDir)
run_cmake(RelAndAbsPath)
run_cmake(Required)

if(CMAKE_SYSTEM_NAME MATCHES "^(Windows|CYGWIN)$")
  run_cmake(WindowsCom)
  run_cmake(WindowsExe)
else()
  # test non readable file only if not root
  execute_process(
    COMMAND id -u $ENV{USER}
    OUTPUT_VARIABLE uid
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(NOT "${uid}" STREQUAL "0")
    run_cmake(ExeNoRead)
  endif()
endif()
