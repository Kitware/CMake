include(RunCMake)

run_cmake(EnvAndHints)
run_cmake(DirsPerName)
run_cmake(NamesPerDir)
run_cmake(RelAndAbsPath)
run_cmake(Required)
run_cmake(NO_CACHE)
run_cmake(IgnorePrefixPath)

if(CMAKE_SYSTEM_NAME MATCHES "^(Windows|CYGWIN|MSYS)$")
  run_cmake(WindowsCom)
  run_cmake(WindowsExe)
else()
  # test non readable file only if not root
  execute_process(
    COMMAND id -u $ENV{USER}
    OUTPUT_VARIABLE uid
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(NOT "${uid}" STREQUAL "0")
    run_cmake(CMP0109-WARN)
    run_cmake(CMP0109-OLD)
    run_cmake(CMP0109-NEW)
  endif()
endif()

if(APPLE)
  run_cmake(BundleSpaceInName)
endif()

run_cmake_with_options(EnvAndHintsDebugVar --debug-find-var=PROG)
