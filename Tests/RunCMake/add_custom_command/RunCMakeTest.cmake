include(RunCMake)

run_cmake(CMP0175-OLD)
run_cmake(CMP0175-WARN)
run_cmake(CMP0175-NEW)
run_cmake(AppendLiteralQuotes)
run_cmake(AppendNoOutput)
run_cmake(AppendNotOutput)
run_cmake(BadArgument)
run_cmake(BadByproduct)
run_cmake(BadOutput)
run_cmake(BadCommand)
run_cmake(ConfigureFile)
run_cmake(GeneratedProperty)
run_cmake(LiteralQuotes)
run_cmake(NoArguments)
run_cmake(NoOutputOrTarget)
run_cmake(OutputAndTarget)
run_cmake(SOURCE)
run_cmake(TargetImported)
run_cmake(TargetLiteralQuotes)
run_cmake(TargetNotInDir)

if(RunCMake_GENERATOR MATCHES "Visual Studio")
  run_cmake(RemoveEmptyCommands)
endif()

run_cmake(AssigningMultipleTargets)
set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/AssigningMultipleTargets-build)
set(RunCMake_TEST_NO_CLEAN 1)
run_cmake_command(AssigningMultipleTargets-build ${CMAKE_COMMAND} --build .)
unset(RunCMake_TEST_BINARY_DIR)
unset(RunCMake_TEST_NO_CLEAN)

if(NOT RunCMake_GENERATOR STREQUAL "Ninja Multi-Config")
  run_cmake(WorkingDirectory)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/WorkingDirectory-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(RunCMake-stdout-file WorkingDirectory-build-multi-config-stdout.txt)
  else()
    set(RunCMake-stdout-file WorkingDirectory-build-single-config-stdout.txt)
  endif()
  run_cmake_command(WorkingDirectory-build ${CMAKE_COMMAND} --build . --config Debug)
  unset(RunCMake-stdout-file)
  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)
endif()

function(test_genex name)
  run_cmake(${name})

  set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${name}-build")
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${name}-build ${CMAKE_COMMAND} --build .)

  if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/wdir/touched")
    message(SEND_ERROR "File not created by target-dependent add_custom_command()!")
  endif()

  unset(RunCMake_TEST_NO_CLEAN)
  unset(RunCMake_TEST_BINARY_DIR)
endfunction()

test_genex(TargetGenexEvent)

if(NOT RunCMake_GENERATOR STREQUAL "Xcode")
  block()
    run_cmake(CommentGenex)
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/CommentGenex-build)
    set(RunCMake_TEST_NO_CLEAN 1)
    run_cmake_command(CommentGenex-build ${CMAKE_COMMAND} --build .)
  endblock()
endif()
