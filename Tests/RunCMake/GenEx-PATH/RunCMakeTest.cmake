
include(RunCMake)

run_cmake(no-arguments)
run_cmake(bad-option)

function(check_path_syntax name test)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${name}-${test}-build)
  set(RunCMake_TEST_VARIANT_DESCRIPTION " - ${name}")
  run_cmake_with_options(${test} ${ARGN})
endfunction()

## Unexpected arguments
### sub-commands with one argument
foreach (subcommand IN ITEMS GET_ROOT_NAME GET_ROOT_DIRECTORY GET_ROOT_PATH GET_FILENAME
                             GET_EXTENSION GET_STEM GET_RELATIVE_PART GET_PARENT_PATH
                             HAS_ROOT_NAME HAS_ROOT_DIRECTORY HAS_ROOT_PATH HAS_FILENAME
                             HAS_EXTENSION HAS_STEM HAS_RELATIVE_PART HAS_PARENT_PATH
                             IS_ABSOLUTE IS_RELATIVE CMAKE_PATH REMOVE_FILENAME REMOVE_EXTENSION
                             NORMAL_PATH)
  check_path_syntax (${subcommand} unexpected-arg "-DPATH_ARGUMENTS=${subcommand},ARG1,ARG2")
endforeach()
foreach (subcommand IN ITEMS GET_EXTENSION GET_STEM REMOVE_EXTENSION)
  if (subcommand STREQUAL "REMOVE_EXTENSION")
    set(RunCMake-stderr-file "unexpected-arg2-stderr.txt")
  endif()
  check_path_syntax ("${subcommand}[LAST_ONLY]" unexpected-arg "-DPATH_ARGUMENTS=${subcommand},LAST_ONLY,ARG1,ARG2")
  unset(RunCMake-stderr-file)
endforeach()
foreach (subcommand IN ITEMS CMAKE_PATH)
  check_path_syntax ("${subcommand}[NORMALIZE]" unexpected-arg "-DPATH_ARGUMENTS=${subcommand},NORMALIZE,ARG1,ARG2")
endforeach()

### sub-commands with two arguments
foreach (subcommand IN ITEMS IS_PREFIX REPLACE_FILENAME REPLACE_EXTENSION RELATIVE_PATH ABSOLUTE_PATH)
  check_path_syntax (${subcommand} unexpected-arg "-DPATH_ARGUMENTS=${subcommand},ARG1,ARG2,ARG3")
endforeach()
foreach (subcommand IN ITEMS IS_PREFIX ABSOLUTE_PATH)
  check_path_syntax ("${subcommand}[NORMALIZE]" unexpected-arg "-DPATH_ARGUMENTS=${subcommand},NORMALIZE,ARG1,ARG2,ARG3")
endforeach()
foreach (subcommand IN ITEMS REPLACE_EXTENSION)
  set(RunCMake-stderr-file "unexpected-arg2-stderr.txt")
  check_path_syntax ("${subcommand}[LAST_ONLY]" unexpected-arg "-DPATH_ARGUMENTS=${subcommand},LAST_ONLY,ARG1,ARG2,ARG3")
  unset(RunCMake-stderr-file)
endforeach()
unset (RunCMake-stderr-file)


function(check_path_execution name)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${name}-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  set(RunCMake_TEST_VARIANT_DESCRIPTION " - ${name}")
  run_cmake_with_options(generate -DPATH_TEST=${name})
  run_cmake_command(check "${CMAKE_COMMAND}" "-DRunCMake_SOURCE_DIR=${RunCMake_SOURCE_DIR}" -P "${RunCMake_TEST_BINARY_DIR}/${name}.cmake")
endfunction()

check_path_execution (GET_ITEM)
check_path_execution (HAS_ITEM)
check_path_execution (CMAKE_PATH)
check_path_execution (APPEND)
check_path_execution (REMOVE_ITEM)
check_path_execution (REPLACE_ITEM)
check_path_execution (NORMAL_PATH)
check_path_execution (RELATIVE_PATH)
check_path_execution (ABSOLUTE_PATH)
check_path_execution (IS_RELATIVE)
check_path_execution (IS_ABSOLUTE)
check_path_execution (IS_PREFIX)
