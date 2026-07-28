include(RunCMake)

run_cmake(Actions)
run_cmake(ExistenceCheck)

run_cmake_with_options(
  CacheInit
  "-DCMAKE_DIAGNOSTIC_INIT=CMD_AUTHOR=IGNORE\;CMD_DEPRECATED=SEND_ERROR"
)

run_cmake_with_options(CommandLine1 -Wno-author -Werror=author)
run_cmake_with_options(CommandLine1 -Werror=author)
run_cmake_with_options(CommandLine1 -Werror=author -Wdeprecated)
run_cmake_with_options(CommandLine1 -Wno-deprecated -Werror=author)
run_cmake_with_options(CommandLine2 -Werror=author -Wno-deprecated)
run_cmake_with_options(CommandLine3 -Wno-error=uninitialized -Winstall-absolute-destination)

function(run_persist_test NAME)
  set(RunCMake_TEST_VARIANT_DESCRIPTION "-first")
  run_cmake_with_options(${NAME} ${ARGN})
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_VARIANT_DESCRIPTION "-second")
  run_cmake(${NAME})
endfunction()

block()
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/Persist-build)
  run_persist_test(Persist1 -Wno-author)
  set(RunCMake_TEST_NO_CLEAN 1)
  run_persist_test(Persist2 -Wauthor)
endblock()

run_cmake_with_options(NonTargetDirectives -Wstrict)
run_cmake_with_options(NonTargetDirectives -Wnon-target-directive)
