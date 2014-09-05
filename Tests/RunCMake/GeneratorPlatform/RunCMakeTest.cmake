include(RunCMake)

run_cmake(NoPlatform)

if("${RunCMake_GENERATOR}" MATCHES "^Visual Studio ([89]|1[0124])( 20[0-9][0-9])?$")
  set(RunCMake_TEST_OPTIONS "-DCMAKE_GENERATOR_PLATFORM=x64")
  run_cmake(x64Platform)
  unset(RunCMake_TEST_OPTIONS)
else()
  set(RunCMake_TEST_OPTIONS "-DCMAKE_GENERATOR_PLATFORM=Bad Platform")
  run_cmake(BadPlatform)
  unset(RunCMake_TEST_OPTIONS)
endif()
