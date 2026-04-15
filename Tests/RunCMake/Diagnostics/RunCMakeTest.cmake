include(RunCMake)

run_cmake(Actions)

run_cmake_with_options(
  CacheInit
  "-DCMAKE_DIAGNOSTIC_INIT=CMD_AUTHOR=IGNORE\;CMD_DEPRECATED=SEND_ERROR"
)

run_cmake_with_options(CommandLine1 -Wno-author -Werror=author)
run_cmake_with_options(CommandLine1 -Werror=author)
run_cmake_with_options(CommandLine1 -Werror=author -Wdeprecated)
run_cmake_with_options(CommandLine1 -Wno-deprecated -Werror=author)
run_cmake_with_options(CommandLine2 -Werror=author -Wno-deprecated)
run_cmake_with_options(CommandLine3 -Wno-error=uninitialized)
