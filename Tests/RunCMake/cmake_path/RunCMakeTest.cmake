include(RunCMake)

# Validate parsing arguments

## input path is not a variable
set (RunCMake-stderr-file "wrong-path-stderr.txt")

### GET sub-command
foreach (subcommand IN ITEMS ROOT_NAME ROOT_DIRECTORY ROOT_PATH FILENAME EXTENSION
                             STEM RELATIVE_PART PARENT_PATH)
  run_cmake_command (GET-${subcommand}-wrong-path "${CMAKE_COMMAND}" "-DCMAKE_PATH_ARGUMENTS=GET wrong_path ${subcommand} output" -P "${RunCMake_SOURCE_DIR}/call-cmake_path.cmake")
endforeach()

foreach (command IN ITEMS APPEND_STRING REMOVE_FILENAME REPLACE_FILENAME
                          REMOVE_EXTENSION REPLACE_EXTENSION NORMAL_PATH
                          RELATIVE_PATH ABSOLUTE_PATH)
  run_cmake_command (${command}-wrong-path "${CMAKE_COMMAND}" "-DCMAKE_PATH_ARGUMENTS=${command} wrong_path" -P "${RunCMake_SOURCE_DIR}/call-cmake_path.cmake")
endforeach()

foreach (command IN ITEMS NATIVE_PATH
                          HAS_ROOT_NAME HAS_ROOT_DIRECTORY HAS_ROOT_PATH
                          HAS_FILENAME HAS_EXTENSION HAS_STEM
                          HAS_RELATIVE_PART HAS_PARENT_PATH
                          IS_ABSOLUTE IS_RELATIVE IS_PREFIX HASH)
  if (command STREQUAL "IS_PREFIX")
    set (extra_args path2)
  else()
    unset (extra_args)
  endif()
  run_cmake_command (${command}-wrong-path "${CMAKE_COMMAND}" "-DCMAKE_PATH_ARGUMENTS=${command} wrong_path ${extra_args} output" -P "${RunCMake_SOURCE_DIR}/call-cmake_path.cmake")
endforeach()


## missing output parameter
set (RunCMake-stderr-file "missing-output-stderr.txt")

### GET sub-command
foreach (subcommand IN ITEMS ROOT_NAME ROOT_DIRECTORY ROOT_PATH FILENAME EXTENSION
                             STEM RELATIVE_PART PARENT_PATH)
  run_cmake_command (GET-${subcommand}-missing-output "${CMAKE_COMMAND}" "-DCMAKE_PATH_ARGUMENTS=GET path ${subcommand}" -P "${RunCMake_SOURCE_DIR}/call-cmake_path.cmake")
endforeach()

### CONVERT sub-command
foreach (subcommand IN ITEMS TO_CMAKE_PATH_LIST TO_NATIVE_PATH_LIST)
  run_cmake_command (CONVERT-${subcommand}-missing-output "${CMAKE_COMMAND}" "-DCMAKE_PATH_ARGUMENTS=CONVERT path ${subcommand}" -P "${RunCMake_SOURCE_DIR}/call-cmake_path.cmake")
endforeach()

### COMPARE sub-command
foreach (subcommand IN ITEMS EQUAL NOT_EQUAL)
  run_cmake_command (COMPARE-${subcommand}-missing-output "${CMAKE_COMMAND}" "-DCMAKE_PATH_ARGUMENTS=COMPARE path ${subcommand} path2" -P "${RunCMake_SOURCE_DIR}/call-cmake_path.cmake")
endforeach()

foreach (command IN ITEMS SET NATIVE_PATH
                          HAS_ROOT_NAME HAS_ROOT_DIRECTORY HAS_ROOT_PATH
                          HAS_FILENAME HAS_EXTENSION HAS_STEM
                          HAS_RELATIVE_PART HAS_PARENT_PATH
                          IS_ABSOLUTE IS_RELATIVE IS_PREFIX HASH)
  if (command STREQUAL "IS_PREFIX")
    set (extra_args path2)
  else()
    unset (extra_args)
  endif()
  run_cmake_command (${command}-missing-output "${CMAKE_COMMAND}" "-DCMAKE_PATH_ARGUMENTS=${command} path ${extra_args}" -P "${RunCMake_SOURCE_DIR}/call-cmake_path.cmake")
endforeach()


## OUTPUT_VARIABLE without argument
set (RunCMake-stderr-file "OUTPUT_VARIABLE-no-arg-stderr.txt")

foreach (command IN ITEMS APPEND APPEND_STRING REMOVE_FILENAME REPLACE_FILENAME
                          REMOVE_EXTENSION REPLACE_EXTENSION NORMAL_PATH
                          RELATIVE_PATH ABSOLUTE_PATH)
  run_cmake_command (${command}-OUTPUT_VARIABLE-no-arg "${CMAKE_COMMAND}" "-DCMAKE_PATH_ARGUMENTS=${command} path OUTPUT_VARIABLE" -P "${RunCMake_SOURCE_DIR}/call-cmake_path.cmake")
endforeach()


## Invalid output variable
set (RunCMake-stderr-file "invalid-output-var-stderr.txt")

### GET sub-command
foreach (subcommand IN ITEMS ROOT_NAME ROOT_DIRECTORY ROOT_PATH FILENAME EXTENSION
                             STEM RELATIVE_PART PARENT_PATH)
  run_cmake_command (GET-${subcommand}-invalid-output "${CMAKE_COMMAND}" "-DCMAKE_PATH_ARGUMENTS=GET path ${subcommand}"  -DCHECK_INVALID_OUTPUT=ON -P "${RunCMake_SOURCE_DIR}/call-cmake_path.cmake")
endforeach()

### CONVERT sub-command
foreach (subcommand IN ITEMS TO_CMAKE_PATH_LIST TO_NATIVE_PATH_LIST)
  run_cmake_command (CONVERT-${subcommand}-invalid-output "${CMAKE_COMMAND}" "-DCMAKE_PATH_ARGUMENTS=CONVERT path ${subcommand}" -DCHECK_INVALID_OUTPUT=ON -P "${RunCMake_SOURCE_DIR}/call-cmake_path.cmake")
endforeach()

### COMPARE sub-command
foreach (subcommand IN ITEMS EQUAL NOT_EQUAL)
  run_cmake_command (COMPARE-${subcommand}-invalid-output "${CMAKE_COMMAND}" "-DCMAKE_PATH_ARGUMENTS=COMPARE path ${subcommand} path2" -DCHECK_INVALID_OUTPUT=ON -P "${RunCMake_SOURCE_DIR}/call-cmake_path.cmake")
endforeach()

foreach (command IN ITEMS NATIVE_PATH
                          HAS_ROOT_NAME HAS_ROOT_DIRECTORY HAS_ROOT_PATH
                          HAS_FILENAME HAS_EXTENSION HAS_STEM
                          HAS_RELATIVE_PART HAS_PARENT_PATH
                          IS_ABSOLUTE IS_RELATIVE IS_PREFIX HASH)
  if (command STREQUAL "IS_PREFIX")
    set (extra_args path2)
  else()
    unset (extra_args)
  endif()
  run_cmake_command (${command}-invalid-output "${CMAKE_COMMAND}" "-DCMAKE_PATH_ARGUMENTS=${command} path ${extra_args}" -DCHECK_INVALID_OUTPUT=ON -P "${RunCMake_SOURCE_DIR}/call-cmake_path.cmake")
endforeach()

foreach (command IN ITEMS APPEND APPEND_STRING REMOVE_FILENAME REPLACE_FILENAME
                          REMOVE_EXTENSION REPLACE_EXTENSION NORMAL_PATH
                          RELATIVE_PATH ABSOLUTE_PATH)
  run_cmake_command (${command}-OUTPUT_VARIABLE-invalid-arg "${CMAKE_COMMAND}" "-DCMAKE_PATH_ARGUMENTS=${command} path OUTPUT_VARIABLE" -DCHECK_INVALID_OUTPUT=ON -P "${RunCMake_SOURCE_DIR}/call-cmake_path.cmake")
endforeach()


## Unexpected arguments
set (RunCMake-stderr-file "unexpected-arg-stderr.txt")

### GET sub-command
foreach (subcommand IN ITEMS ROOT_NAME ROOT_DIRECTORY ROOT_PATH FILENAME EXTENSION
                             STEM RELATIVE_PART PARENT_PATH)
  if (subcommand STREQUAL "EXTENSION" OR subcommand STREQUAL "STEM")
    set (extra_args LAST_ONLY)
  else()
    unset (extra_args)
  endif()
  run_cmake_command (GET-${subcommand}-unexpected-arg "${CMAKE_COMMAND}" "-DCMAKE_PATH_ARGUMENTS=GET path ${subcommand} ${extra_args} unexpected output" -P "${RunCMake_SOURCE_DIR}/call-cmake_path.cmake")
endforeach()

### CONVERT sub-command
foreach (subcommand IN ITEMS TO_CMAKE_PATH_LIST TO_NATIVE_PATH_LIST)
  run_cmake_command (CONVERT-${subcommand}-unexpected-arg "${CMAKE_COMMAND}" "-DCMAKE_PATH_ARGUMENTS=CONVERT path ${subcommand} output unexpected" -P "${RunCMake_SOURCE_DIR}/call-cmake_path.cmake")
endforeach()

foreach (command IN ITEMS REMOVE_FILENAME REPLACE_FILENAME
                          REMOVE_EXTENSION REPLACE_EXTENSION NORMAL_PATH
                          RELATIVE_PATH ABSOLUTE_PATH)
  if (command STREQUAL "REPLACE_FILENAME" OR command STREQUAL "REPLACE_EXTENSION")
    set (extra_args input)
  else()
    unset (extra_args)
  endif()
  run_cmake_command (${command}-unexpected-arg "${CMAKE_COMMAND}" "-DCMAKE_PATH_ARGUMENTS=${command} path ${extra_args} unexpected" -P "${RunCMake_SOURCE_DIR}/call-cmake_path.cmake")
endforeach()

foreach (command IN ITEMS SET NATIVE_PATH
                          HAS_ROOT_NAME HAS_ROOT_DIRECTORY HAS_ROOT_PATH
                          HAS_FILENAME HAS_EXTENSION HAS_STEM
                          HAS_RELATIVE_PART HAS_PARENT_PATH
                          IS_ABSOLUTE IS_RELATIVE IS_PREFIX
                          HASH)
  if (command STREQUAL "IS_PREFIX")
    set (extra_args input)
  else()
    unset (extra_args)
  endif()
  run_cmake_command (${command}-unexpected-arg "${CMAKE_COMMAND}" "-DCMAKE_PATH_ARGUMENTS=${command} path ${extra_args} unexpected output" -P "${RunCMake_SOURCE_DIR}/call-cmake_path.cmake")
endforeach()
unset (RunCMake-stderr-file)

run_cmake(GET-wrong-operator)
run_cmake(CONVERT-wrong-operator)
run_cmake(COMPARE-wrong-operator)

set (RunCMake_TEST_OPTIONS "-DRunCMake_SOURCE_DIR=${RunCMake_SOURCE_DIR}")

run_cmake(GET)
run_cmake(SET)
run_cmake(APPEND)
run_cmake(APPEND_STRING)
run_cmake(REMOVE_FILENAME)
run_cmake(REPLACE_FILENAME)
run_cmake(REMOVE_EXTENSION)
run_cmake(REPLACE_EXTENSION)
run_cmake(NORMAL_PATH)
run_cmake(RELATIVE_PATH)
run_cmake(ABSOLUTE_PATH)
run_cmake(NATIVE_PATH)
run_cmake(CONVERT)
run_cmake(COMPARE)
run_cmake(HAS_ITEM)
run_cmake(IS_ABSOLUTE)
run_cmake(IS_RELATIVE)
run_cmake(IS_PREFIX)
run_cmake(HASH)
