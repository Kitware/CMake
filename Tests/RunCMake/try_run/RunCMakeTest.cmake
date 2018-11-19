include(RunCMake)

run_cmake(BadLinkLibraries)

if (CMAKE_SYSTEM_NAME MATCHES "^(Linux|Darwin|Windows)$" AND
    CMAKE_C_COMPILER_ID MATCHES "^(MSVC|GNU|Clang|AppleClang)$")
  set (RunCMake_TEST_OPTIONS -DRunCMake_C_COMPILER_ID=${CMAKE_C_COMPILER_ID})
  run_cmake(LinkOptions)
  unset (RunCMake_TEST_OPTIONS)
endif()
