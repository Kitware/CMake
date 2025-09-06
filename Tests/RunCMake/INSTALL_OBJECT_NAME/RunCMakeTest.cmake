cmake_minimum_required(VERSION 3.16)

include(RunCMake)

# This test does installation of `OBJECT` libraries which does not work with
# multi-arch compilation under Xcode.
if (RunCMake_GENERATOR STREQUAL "Xcode" AND "$ENV{CMAKE_OSX_ARCHITECTURES}" MATCHES "[;$]")
  return ()
endif ()

function(run_install_test case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)
  set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE:STRING=Debug "-DCMAKE_INSTALL_PREFIX:PATH=${RunCMake_TEST_BINARY_DIR}/fake_install")
  run_cmake(${case})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${case}-build ${CMAKE_COMMAND} --build . --config Debug)
  set(prefix "${RunCMake_TEST_BINARY_DIR}/real_install")
  run_cmake_command(${case}-install ${CMAKE_COMMAND} --install . --config Debug --prefix "${prefix}")

  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/Consume-${case}-build)
  set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE:STRING=Debug "-DCMAKE_PREFIX_PATH:PATH=${prefix}")
  run_cmake(Consume-${case} "-DCMAKE_PREFIX_PATH=${prefix}")
  run_cmake_command(Consume-${case}-build ${CMAKE_COMMAND} --build . --config Debug)
  run_cmake_command(Consume-${case}-test ${CMAKE_CTEST_COMMAND} -C Debug)
endfunction()

function (check_installed_object path)
  if (NOT EXISTS "${path}")
    list(APPEND RunCMake_TEST_FAILED
      "Could not find installed object at '${path}'")
  endif ()
  set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
endfunction ()

run_cmake(Inspect)
include("${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")

run_cmake(NoAbsolute)
run_cmake(NoDotDot)
run_cmake(InvalidGeneratorExpression)
run_cmake(UnsupportedGeneratorExpressionHeadTarget)
run_cmake(UnsupportedGeneratorExpressionLinkLanguage)

run_install_test(ObjectLibrary)
run_install_test(DisableOnEmpty)
if (RunCMake_GENERATOR MATCHES "(Ninja|Makefiles|Visual Studio)")
  run_install_test(ShortInstallStrategy)
endif ()
run_install_test(GeneratorExpression)
run_install_test(GeneratorExpressionContextSensitive)
