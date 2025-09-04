cmake_minimum_required(VERSION 3.16)

include(RunCMake)

# This test does installation of `OBJECT` libraries which does not work with
# multi-arch compilation under Xcode.
if (RunCMake_GENERATOR STREQUAL "Xcode" AND "$ENV{CMAKE_OSX_ARCHITECTURES}" MATCHES "[;$]")
  return ()
endif ()

if (RunCMake_GENERATOR STREQUAL "FASTBuild")
  # FASTBuild does not offer full control over object paths. Just skip all
  # tests rather than expecting some half-supported behavior.
  return ()
endif ()

if (RunCMake_GENERATOR STREQUAL "Xcode")
  # Xcode does not offer full control over object paths. Just skip all tests
  # rather than expecting some half-supported behavior.
  return ()
endif ()

function(run_build_test case)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${case}-build)
  set(RunCMake_TEST_OPTIONS -DCMAKE_BUILD_TYPE:STRING=Debug)
  run_cmake(${case})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${case}-build ${CMAKE_COMMAND} --build . --config Debug)
endfunction()

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

function (check_build_object target objname)
  set(cmf_subdir "CMakeFiles/")
  set(config_subdir "")
  if (RunCMake_GENERATOR MATCHES "Visual Studio")
    set(cmf_subdir "")
  endif ()
  if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(config_subdir "Debug/")
  endif ()
  set(path "${RunCMake_TEST_BINARY_DIR}/${cmf_subdir}${target}.dir/${config_subdir}${objname}${CMAKE_C_OUTPUT_EXTENSION}")
  if (NOT EXISTS "${path}")
    list(APPEND RunCMake_TEST_FAILED
      "Could not find built object at '${path}'")
  endif ()
  set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
endfunction ()

function (check_short_build_object target objname)
  set(config_subdir "")
  if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(config_subdir "Debug/")
  endif ()
  set(obj_subdir ".o")
  if (RunCMake_GENERATOR MATCHES "(Borland Makefiles|Watcom WMake)")
    set(obj_subdir "_o")
  endif ()
  set(path "${RunCMake_TEST_BINARY_DIR}/${obj_subdir}/${target}/${config_subdir}${objname}${CMAKE_C_OUTPUT_EXTENSION}")
  if (NOT EXISTS "${path}")
    list(APPEND RunCMake_TEST_FAILED
      "Could not find built object at '${path}'")
  endif ()
  set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
endfunction ()

function (check_installed_object path)
  if (NOT EXISTS "${path}")
    list(APPEND RunCMake_TEST_FAILED
      "Could not find installed object at '${path}'")
  endif ()
  set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
endfunction ()

run_cmake(Inspect)
include("${RunCMake_BINARY_DIR}/Inspect-build/info.cmake")

run_build_test(Executable)
run_build_test(SharedLibrary)
run_build_test(StaticLibrary)

if (RunCMake_GENERATOR STREQUAL "Ninja")
  run_cmake(Collision)
else ()
  run_build_test(CollisionBuild)
endif ()

run_cmake(NoAbsolute)
run_cmake(NoDotDot)
run_cmake(InvalidGeneratorExpression)
run_cmake(UnsupportedGeneratorExpressionContextSensitive)
run_cmake(UnsupportedGeneratorExpressionHeadTarget)
run_cmake(UnsupportedGeneratorExpressionLinkLanguage)

run_install_test(ObjectLibrary)
run_install_test(DisableOnEmpty)
if (RunCMake_GENERATOR MATCHES "(Ninja|Makefiles|Visual Studio)")
  run_install_test(ShortStrategy)
  run_install_test(ShortInstallStrategy)
endif ()
run_install_test(GeneratorExpression)
