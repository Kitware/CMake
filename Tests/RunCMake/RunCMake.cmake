foreach(
  arg
  IN ITEMS
    RunCMake_GENERATOR
    RunCMake_SOURCE_DIR
    RunCMake_BINARY_DIR
  )
  if(NOT DEFINED ${arg})
    message(FATAL_ERROR "${arg} not given!")
  endif()
endforeach()

function(run_cmake test)
  if(DEFINED ENV{RunCMake_TEST_FILTER})
    set(test_and_variant "${test}${RunCMake_TEST_VARIANT_DESCRIPTION}")
    if(NOT test_and_variant MATCHES "$ENV{RunCMake_TEST_FILTER}")
      return()
    endif()
    unset(test_and_variant)
  endif()

  set(top_src "${RunCMake_SOURCE_DIR}")
  set(top_bin "${RunCMake_BINARY_DIR}")
  if(EXISTS ${top_src}/${test}-result.txt)
    file(READ ${top_src}/${test}-result.txt expect_result)
    string(REGEX REPLACE "\n+$" "" expect_result "${expect_result}")
  elseif(DEFINED RunCMake_TEST_EXPECT_RESULT)
    set(expect_result "${RunCMake_TEST_EXPECT_RESULT}")
  else()
    set(expect_result 0)
  endif()

  string(TOLOWER ${CMAKE_HOST_SYSTEM_NAME} platform_name)
  #remove all additional bits from cygwin/msys name
  if(platform_name MATCHES cygwin)
    set(platform_name cygwin)
  endif()
  if(platform_name MATCHES msys)
    set(platform_name msys)
  endif()

  foreach(o IN ITEMS stdout stderr config)
    if(RunCMake-${o}-file AND EXISTS ${top_src}/${RunCMake-${o}-file})
      file(READ ${top_src}/${RunCMake-${o}-file} expect_${o})
      string(REGEX REPLACE "\n+$" "" expect_${o} "${expect_${o}}")
    elseif(EXISTS ${top_src}/${test}-${o}-${platform_name}.txt)
      file(READ ${top_src}/${test}-${o}-${platform_name}.txt expect_${o})
      string(REGEX REPLACE "\n+$" "" expect_${o} "${expect_${o}}")
    elseif(EXISTS ${top_src}/${test}-${o}.txt)
      file(READ ${top_src}/${test}-${o}.txt expect_${o})
      string(REGEX REPLACE "\n+$" "" expect_${o} "${expect_${o}}")
    elseif(DEFINED RunCMake_TEST_EXPECT_${o})
      string(REGEX REPLACE "\n+$" "" expect_${o} "${RunCMake_TEST_EXPECT_${o}}")
    else()
      unset(expect_${o})
    endif()
  endforeach()
  foreach(o IN ITEMS stdout stderr config)
    if(DEFINED RunCMake_TEST_NOT_EXPECT_${o})
      string(REGEX REPLACE "\n+$" "" not_expect_${o} "${RunCMake_TEST_NOT_EXPECT_${o}}")
    endif()
  endforeach()
  if (NOT expect_stderr)
    if (NOT RunCMake_DEFAULT_stderr)
      set(RunCMake_DEFAULT_stderr "^$")
    endif()
    set(expect_stderr ${RunCMake_DEFAULT_stderr})
  endif()

  if (NOT RunCMake_TEST_SOURCE_DIR)
    set(RunCMake_TEST_SOURCE_DIR "${top_src}")
  endif()
  if(NOT RunCMake_TEST_BINARY_DIR)
    set(RunCMake_TEST_BINARY_DIR "${top_bin}/${test}-build")
  endif()
  if(NOT RunCMake_TEST_NO_CLEAN)
    file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  endif()
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  if(RunCMake-prep-file AND EXISTS ${top_src}/${RunCMake-prep-file})
    include(${top_src}/${RunCMake-prep-file})
  else()
    include(${top_src}/${test}-prep.cmake OPTIONAL)
  endif()
  if(RunCMake_TEST_OUTPUT_MERGE)
    set(actual_stderr_var actual_stdout)
    set(actual_stderr "")
  else()
    set(actual_stderr_var actual_stderr)
  endif()
  if(DEFINED RunCMake_TEST_TIMEOUT)
    set(maybe_timeout TIMEOUT ${RunCMake_TEST_TIMEOUT})
  else()
    set(maybe_timeout "")
  endif()
  if(RunCMake-stdin-file AND EXISTS ${top_src}/${RunCMake-stdin-file})
    set(maybe_input_file INPUT_FILE ${top_src}/${RunCMake-stdin-file})
  elseif(EXISTS ${top_src}/${test}-stdin.txt)
    set(maybe_input_file INPUT_FILE ${top_src}/${test}-stdin.txt)
  else()
    set(maybe_input_file "")
  endif()
  if(NOT RunCMake_TEST_COMMAND)
    if(NOT DEFINED RunCMake_TEST_OPTIONS)
      set(RunCMake_TEST_OPTIONS "")
    endif()
    if(APPLE)
      list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_POLICY_DEFAULT_CMP0025=NEW)
    endif()
    if(RunCMake_TEST_LCC AND NOT RunCMake_TEST_NO_CMP0129)
      list(APPEND RunCMake_TEST_OPTIONS -DCMAKE_POLICY_DEFAULT_CMP0129=NEW)
    endif()
    if(RunCMake_MAKE_PROGRAM)
      list(APPEND RunCMake_TEST_OPTIONS "-DCMAKE_MAKE_PROGRAM=${RunCMake_MAKE_PROGRAM}")
    endif()
    set(RunCMake_TEST_COMMAND ${CMAKE_COMMAND})
    if(NOT RunCMake_TEST_NO_SOURCE_DIR)
      list(APPEND RunCMake_TEST_COMMAND "${RunCMake_TEST_SOURCE_DIR}")
    endif()
    list(APPEND RunCMake_TEST_COMMAND -G "${RunCMake_GENERATOR}")
    if(RunCMake_GENERATOR_PLATFORM)
      list(APPEND RunCMake_TEST_COMMAND -A "${RunCMake_GENERATOR_PLATFORM}")
    endif()
    if(RunCMake_GENERATOR_TOOLSET)
      list(APPEND RunCMake_TEST_COMMAND -T "${RunCMake_GENERATOR_TOOLSET}")
    endif()
    if(RunCMake_GENERATOR_INSTANCE)
      list(APPEND RunCMake_TEST_COMMAND "-DCMAKE_GENERATOR_INSTANCE=${RunCMake_GENERATOR_INSTANCE}")
    endif()
    list(APPEND RunCMake_TEST_COMMAND
      -DRunCMake_TEST=${test}
      --no-warn-unused-cli
      )
  else()
    set(RunCMake_TEST_OPTIONS "")
  endif()
  if(NOT DEFINED RunCMake_TEST_RAW_ARGS)
    set(RunCMake_TEST_RAW_ARGS "")
  endif()
  if(NOT RunCMake_TEST_COMMAND_WORKING_DIRECTORY)
    set(RunCMake_TEST_COMMAND_WORKING_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  endif()
  string(CONCAT _code [[execute_process(
    COMMAND ${RunCMake_TEST_COMMAND}
            ${RunCMake_TEST_OPTIONS}
            ]] "${RunCMake_TEST_RAW_ARGS}\n" [[
    WORKING_DIRECTORY "${RunCMake_TEST_COMMAND_WORKING_DIRECTORY}"
    OUTPUT_VARIABLE actual_stdout
    ERROR_VARIABLE ${actual_stderr_var}
    RESULT_VARIABLE actual_result
    ENCODING UTF8
    ${maybe_timeout}
    ${maybe_input_file}
    )]])
  if(DEFINED ENV{PWD})
    set(old_pwd "$ENV{PWD}")
  else()
    set(old_pwd)
  endif()
  # Emulate a shell using this directory.
  set(ENV{PWD} "${RunCMake_TEST_COMMAND_WORKING_DIRECTORY}")
  cmake_language(EVAL CODE "${_code}")
  if(DEFINED old_pwd)
    set(ENV{PWD} "${old_pwd}")
  else()
    set(ENV{PWD})
  endif()
  set(msg "")
  if(NOT "${actual_result}" MATCHES "${expect_result}")
    string(APPEND msg "Result is [${actual_result}], not [${expect_result}].\n")
  endif()
  set(config_file "${RunCMake_TEST_COMMAND_WORKING_DIRECTORY}/CMakeFiles/CMakeConfigureLog.yaml")
  if(EXISTS "${config_file}")
    file(READ "${config_file}" actual_config)
  else()
    set(actual_config "")
  endif()

  # Special case: remove ninja no-op line from stderr, but not stdout.
  # Test cases that look for it should use RunCMake_TEST_OUTPUT_MERGE.
  string(REGEX REPLACE "(^|\r?\n)ninja: no work to do\\.\r?\n" "\\1" actual_stderr "${actual_stderr}")

  # Remove incidental content from both stdout and stderr.
  string(CONCAT ignore_line_regex
    "(^|\n)((==[0-9]+=="
    "|BullseyeCoverage"
    "|[a-z]+\\([0-9]+\\) malloc:"
    "|clang[^:]*: warning: the object size sanitizer has no effect at -O0, but is explicitly enabled:"
    "|flang-new: warning: argument unused during compilation: .-flang-experimental-exec."
    "|icp?x: remark: Note that use of .-g. without any optimization-level option will turn off most compiler optimizations"
    "|ifx: remark #10440: Note that use of a debug option without any optimization-level option will turnoff most compiler optimizations"
    "|lld-link: warning: procedure symbol record for .* refers to PDB item index [0-9A-Fa-fx]+ which is not a valid function ID record"
    "|Error kstat returned"
    "|Hit xcodebuild bug"
    "|Recompacting log\\.\\.\\."

    "|LICENSE WARNING:"
    "|Your license to use PGI[^\n]*expired"
    "|Please obtain a new version at"
    "|contact PGI Sales at"
    "|ic(p?c|l): remark #10441: The Intel\\(R\\) C\\+\\+ Compiler Classic \\(ICC\\) is deprecated"

    "|[^\n]*install_name_tool: warning: changes being made to the file will invalidate the code signature in:"
    "|[^\n]*(createItemModels|_NSMainThread|Please file a bug at)"
    "|[^\n]*xcodebuild[^\n]*DVTAssertions: Warning"
    "|[^\n]*xcodebuild[^\n]*DVTCoreDeviceEnabledState: DVTCoreDeviceEnabledState_Disabled set via user default"
    "|[^\n]*xcodebuild[^\n]*DVTPlugInManager"
    "|[^\n]*xcodebuild[^\n]*DVTSDK: Warning: SDK path collision for path"
    "|[^\n]*xcodebuild[^\n]*Requested but did not find extension point with identifier"
    "|[^\n]*xcodebuild[^\n]*nil host used in call to allows.*HTTPSCertificateForHost"
    "|[^\n]*xcodebuild[^\n]*warning: file type[^\n]*is based on missing file type"
    "|[^\n]*objc[^\n]*: Class [^\n]* One of the two will be used. Which one is undefined."
    "|[^\n]*is a member of multiple groups"
    "|[^\n]*offset in archive not a multiple of 8"
    "|[^\n]*from Time Machine by path"
    "|[^\n]*Bullseye Testing Technology"
    ")[^\n]*\n)+"
    )
  if(RunCMake_IGNORE_POLICY_VERSION_DEPRECATION)
    string(REGEX REPLACE [[
^CMake Deprecation Warning at [^
]*CMakeLists.txt:1 \(cmake_minimum_required\):
  Compatibility with CMake < 3\.5 will be removed from a future version of
  CMake.

  Update the VERSION argument <min> value or use a \.\.\.<max> suffix to tell
  CMake that the project does not need compatibility with older versions\.
+
]] "" actual_stderr "${actual_stderr}")
  endif()
  foreach(o IN ITEMS stdout stderr config)
    string(REGEX REPLACE "\r\n" "\n" actual_${o} "${actual_${o}}")
    string(REGEX REPLACE "${ignore_line_regex}" "\\1" actual_${o} "${actual_${o}}")
    string(REGEX REPLACE "\n+$" "" actual_${o} "${actual_${o}}")
    if(DEFINED expect_${o})
      if(NOT "${actual_${o}}" MATCHES "${expect_${o}}")
        string(APPEND msg "${o} does not match that expected.\n")
      endif()
    endif()
    if(DEFINED not_expect_${o})
      if("${actual_${o}}" MATCHES "${not_expect_${o}}")
        string(APPEND msg "${o} matches that not expected.\n")
      endif()
    endif()
  endforeach()
  unset(RunCMake_TEST_FAILED)
  if(RunCMake-check-file AND EXISTS ${top_src}/${RunCMake-check-file})
    include(${top_src}/${RunCMake-check-file})
  else()
    include(${top_src}/${test}-check.cmake OPTIONAL)
  endif()
  if(RunCMake_TEST_FAILED)
    set(msg "${RunCMake_TEST_FAILED}\n${msg}")
  endif()
  if(msg)
    string(REPLACE ";" "\" \"" command "\"${RunCMake_TEST_COMMAND}\"")
    if(RunCMake_TEST_OPTIONS)
      string(REPLACE ";" "\" \"" options "\"${RunCMake_TEST_OPTIONS}\"")
      string(APPEND command " ${options}")
    endif()
    if(RunCMake_TEST_RAW_ARGS)
      string(APPEND command " ${RunCMake_TEST_RAW_ARGS}")
    endif()
    string(APPEND msg "Command was:\n command> ${command}\n")
  endif()
  if(msg)
    foreach(o IN ITEMS stdout stderr config)
      if(DEFINED expect_${o})
        string(REGEX REPLACE "\n" "\n expect-${o}> " expect_${o} " expect-${o}> ${expect_${o}}")
        string(APPEND msg "Expected ${o} to match:\n${expect_${o}}\n")
      endif()
      if(NOT o STREQUAL "config" OR DEFINED expect_${o})
        string(REGEX REPLACE "\n" "\n actual-${o}> " actual_${o} " actual-${o}> ${actual_${o}}")
        string(APPEND msg "Actual ${o}:\n${actual_${o}}\n")
      endif()
    endforeach()
    message(SEND_ERROR "${test}${RunCMake_TEST_VARIANT_DESCRIPTION} - FAILED:\n${msg}")
  else()
    message(STATUS "${test}${RunCMake_TEST_VARIANT_DESCRIPTION} - PASSED")
  endif()
endfunction()

function(run_cmake_command test)
  set(RunCMake_TEST_COMMAND "${ARGN}")
  run_cmake(${test})
endfunction()

function(run_cmake_script test)
  set(RunCMake_TEST_COMMAND ${CMAKE_COMMAND} ${ARGN} -P ${RunCMake_SOURCE_DIR}/${test}.cmake)
  run_cmake(${test})
endfunction()

function(run_cmake_with_options test)
  set(RunCMake_TEST_OPTIONS "${ARGN}")
  run_cmake(${test})
endfunction()

function(run_cmake_with_raw_args test args)
  set(RunCMake_TEST_RAW_ARGS "${args}")
  run_cmake(${test})
endfunction()

function(ensure_files_match expected_file actual_file)
  if(NOT EXISTS "${expected_file}")
    message(FATAL_ERROR "Expected file does not exist:\n  ${expected_file}")
  endif()
  if(NOT EXISTS "${actual_file}")
    message(FATAL_ERROR "Actual file does not exist:\n  ${actual_file}")
  endif()
  file(READ "${expected_file}" expected_file_content)
  file(READ "${actual_file}" actual_file_content)
  if(NOT "${expected_file_content}" STREQUAL "${actual_file_content}")
    message(FATAL_ERROR "Actual file content does not match expected:\n
    \n
      expected file: ${expected_file}\n
      expected content:\n
      ${expected_file_content}\n
    \n
      actual file: ${actual_file}\n
      actual content:\n
      ${actual_file_content}\n
    ")
  endif()
endfunction()

# Get the user id on unix if possible.
function(get_unix_uid var)
  set("${var}" "" PARENT_SCOPE)
  if(UNIX)
    set(ID "id")
    if(CMAKE_SYSTEM_NAME STREQUAL "SunOS" AND EXISTS "/usr/xpg4/bin/id")
      set (ID "/usr/xpg4/bin/id")
    endif()
    execute_process(COMMAND ${ID} -u $ENV{USER} OUTPUT_VARIABLE uid ERROR_QUIET
                    RESULT_VARIABLE status OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(status EQUAL 0)
      set("${var}" "${uid}" PARENT_SCOPE)
    endif()
  endif()
endfunction()

# Protect RunCMake tests from calling environment.
unset(ENV{MAKEFLAGS})
