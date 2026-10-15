include(RunCMake)

function(run_test name)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${name}-build)
  run_cmake(${name})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${name}-build ${CMAKE_COMMAND} --build . --config Debug)
  run_cmake_command(${name}-test ${CMAKE_CTEST_COMMAND} -C Debug)
endfunction()

run_cmake(DisabledPch)
block()
  set(ENV{CMAKE_DISABLE_PRECOMPILE_HEADERS} "ON")
  run_cmake(DisabledPchEnv)
  unset(ENV{CMAKE_DISABLE_PRECOMPILE_HEADERS})
endblock()
run_cmake(PchDebugGenex)
if (RunCMake_GENERATOR MATCHES "(Ninja|Makefiles|Visual Studio)")
  run_cmake(PchDebugGenexShort)
endif ()
run_test(PchInterface)
if (RunCMake_GENERATOR MATCHES "(Ninja|Makefiles|Visual Studio)")
  run_test(PchInterfaceShort)
endif ()
run_test(PchInterfaceUnity)
run_cmake(PchPrologueEpilogue)
run_test(SkipPrecompileHeaders)
run_test(CXXnotC)
run_cmake(PchMultilanguage)
if(RunCMake_GENERATOR MATCHES "Make|Ninja")
  run_cmake(PchWarnInvalid)

  if(CMAKE_C_COMPILER_ID STREQUAL "Clang" AND
      CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 11.0.0 AND
      NOT CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
    run_cmake(PchInstantiateTemplates)
  endif()
endif()
# Only the Xcode generator passes the precompiled header to Clang as a prefix
# header, which the compiler honors only when its '-include' flag comes first.
# The other generators use '-include-pch' instead and are unaffected, except
# on multi-architecture Apple builds, where they have the same limitation.
# Only Clang-based compilers take the '-Xclang -include<file>' spelling that
# the Xcode generator uses to keep a force include out of the driver's way.
if(RunCMake_GENERATOR STREQUAL "Xcode" AND
    CMAKE_C_COMPILER_ID MATCHES "^(Clang|AppleClang|IntelLLVM)$")
  run_test(PchForceInclude)
endif()
run_test(PchIncludedAllLanguages)
run_test(PchIncludedOneLanguage)
run_test(PchLibObjLibExe)
