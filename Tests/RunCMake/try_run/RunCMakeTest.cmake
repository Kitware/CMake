include(RunCMake)

run_cmake(BadLinkLibraries)
run_cmake(BinDirEmpty)
run_cmake(BinDirRelative)

if (CMAKE_SYSTEM_NAME MATCHES "^(Linux|Darwin|Windows)$" AND
    CMAKE_C_COMPILER_ID MATCHES "^(MSVC|GNU|LCC|Clang|AppleClang)$")
  set (RunCMake_TEST_OPTIONS -DRunCMake_C_COMPILER_ID=${CMAKE_C_COMPILER_ID})
  run_cmake(LinkOptions)
  unset (RunCMake_TEST_OPTIONS)
endif()

run_cmake(WorkingDirArg)

run_cmake(NoOutputVariable)
run_cmake(NoCompileOutputVariable)
run_cmake(NoRunOutputVariable)
run_cmake(NoRunStdOutVariable)
run_cmake(NoRunStdErrVariable)
run_cmake(NoWorkingDirectory)
