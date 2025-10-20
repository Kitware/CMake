
include(RunCMake)

run_cmake(no-arguments)
run_cmake(bad-option)

function(check_string_syntax name test)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${name}-${test}-build)
  set(RunCMake_TEST_VARIANT_DESCRIPTION " - ${name}")
  run_cmake_with_options(${test} ${ARGN})
endfunction()

## Unexpected arguments
### sub-commands with one argument
foreach (subcommand IN ITEMS LENGTH TOLOWER TOUPPER HEX MAKE_C_IDENTIFIER)
  check_string_syntax (${subcommand} unexpected-arg "-DSTRING_ARGUMENTS=${subcommand},ARG1,ARG2")
endforeach()

### sub-commands with two arguments
foreach (subcommand IN ITEMS HASH STRIP QUOTE)
  check_string_syntax (${subcommand} unexpected-arg "-DSTRING_ARGUMENTS=${subcommand},ARG1,ARG2,ARG3")
endforeach()

### sub-commands with three arguments
foreach (subcommand IN ITEMS SUBSTRING)
  check_string_syntax (${subcommand} unexpected-arg "-DSTRING_ARGUMENTS=${subcommand},ARG1,ARG2,ARG3,ARG4")
endforeach()
foreach (subcommand IN ITEMS FIND MATCH RANDOM)
  check_string_syntax (${subcommand} unexpected-arg2 "-DSTRING_ARGUMENTS=${subcommand},ARG1,ARG2,ARG3,ARG4")
endforeach()

### sub-commands with four arguments
foreach (subcommand IN ITEMS REPLACE)
  check_string_syntax (${subcommand} unexpected-arg2 "-DSTRING_ARGUMENTS=${subcommand},ARG1,ARG2,ARG3,ARG4,ARG5")
endforeach()

run_cmake(SUBSTRING-WrongArguments)
run_cmake(FIND-WrongArguments)
run_cmake(MATCH-WrongArguments)
run_cmaKE(ASCII-WrongArguments)
run_cmake(TIMESTAMP-WrongArguments)
run_cmake(RANDOM-WrongArguments)
run_cmake(UUID-WrongArguments)
run_cmake(REPLACE-WrongArguments)
run_cmake(STRIP-WrongArguments)
run_cmake(QUOTE-WrongArguments)
run_cmake(HASH-WrongArguments)


function(check_string_execution name)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${name}-build)
  if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(CONFIG_OPTION -DCMAKE_CONFIGURATION_TYPES=Debug)
  else()
    set(CONFIG_OPTION -DCMAKE_BUILD_TYPE=Debug)
  endif()
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_VARIANT_DESCRIPTION " - ${name}")
  run_cmake_with_options(generate -DSTRING_TEST=${name} ${CONFIG_OPTION})
  run_cmake_command(check "${CMAKE_COMMAND}" "-DRunCMake_SOURCE_DIR=${RunCMake_SOURCE_DIR}" -P "${RunCMake_TEST_BINARY_DIR}/${name}.cmake")
endfunction()

check_string_execution (LENGTH)
check_string_execution (SUBSTRING)
check_string_execution (FIND)
check_string_execution (MATCH)
check_string_execution (JOIN)
check_string_execution (ASCII)
check_string_execution (TIMESTAMP)
check_string_execution (RANDOM)
check_string_execution (UUID)
check_string_execution (REPLACE)
check_string_execution (APPEND)
check_string_execution (PREPEND)
check_string_execution (TOLOWER)
check_string_execution (TOUPPER)
check_string_execution (STRIP)
check_string_execution (QUOTE)
check_string_execution (HEX)
check_string_execution (HASH)
