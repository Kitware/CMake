include(RunCMake)

if("${RunCMake_GENERATOR}" MATCHES "^Visual Studio 1[56789]")
  set(RunCMake_GENERATOR_INSTANCE "")
  run_cmake(DefaultInstance)

  set(RunCMake_GENERATOR_INSTANCE "${RunCMake_SOURCE_DIR}/instance_does_not_exist")
  run_cmake(MissingInstance)
  set(RunCMake_TEST_OPTIONS -DCMAKE_TOOLCHAIN_FILE=${RunCMake_SOURCE_DIR}/MissingInstance-toolchain.cmake)
  run_cmake(MissingInstanceToolchain)
  unset(RunCMake_TEST_OPTIONS)

  set(RunCMake_GENERATOR_INSTANCE "Test Instance,nocomma")
  run_cmake(BadFieldNoComma)
  set(RunCMake_GENERATOR_INSTANCE "Test Instance,unknown=")
  run_cmake(BadFieldUnknown)
else()
  set(RunCMake_GENERATOR_INSTANCE "")
  run_cmake(NoInstance)

  set(RunCMake_GENERATOR_INSTANCE "Bad Instance")
  run_cmake(BadInstance)

  set(RunCMake_TEST_OPTIONS -DCMAKE_TOOLCHAIN_FILE=${RunCMake_SOURCE_DIR}/BadInstance-toolchain.cmake)
  run_cmake(BadInstanceToolchain)
  unset(RunCMake_TEST_OPTIONS)
endif()
