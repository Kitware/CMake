# Check whether the C++ source of a mixed target is compiled with
# /Zc:__STDC__ like its C sources.

# Only the NEW behavior with extensions off passes the flag.
# The C++ sources get the flag only when the C sources do, so not for
# C standards that do not support it, such as C99.
if(test MATCHES "-NEW-mixed-ext-off-std99")
  set(expect_flag FALSE)
elseif(test MATCHES "-NEW-mixed-ext-off-std")
  set(expect_flag TRUE)
else()
  set(expect_flag FALSE)
endif()

function(check_flag what command)
  if(command MATCHES "[-/]Zc:__STDC__")
    set(has_flag TRUE)
  else()
    set(has_flag FALSE)
  endif()
  if(expect_flag AND NOT has_flag)
    string(APPEND RunCMake_TEST_FAILED
      "${what} is not compiled with /Zc:__STDC__:\n  ${command}\n")
  elseif(NOT expect_flag AND has_flag)
    string(APPEND RunCMake_TEST_FAILED
      "${what} is compiled with /Zc:__STDC__ unexpectedly:\n  ${command}\n")
  endif()
  set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
endfunction()

if(RunCMake_GENERATOR MATCHES "^Visual Studio")
  # The Visual Studio generators compile the C and C++ sources with the
  # options of one ClCompile, so the flag is in its AdditionalOptions.
  set(vcxproj "${RunCMake_TEST_BINARY_DIR}/main.vcxproj")
  if(NOT EXISTS "${vcxproj}")
    string(APPEND RunCMake_TEST_FAILED "${vcxproj} does not exist\n")
    return()
  endif()
  file(STRINGS "${vcxproj}" options REGEX "<AdditionalOptions>")
  check_flag("main.vcxproj" "${options}")
  return()
endif()

# Other generators export the compile command of each source.
set(compile_commands "${RunCMake_TEST_BINARY_DIR}/compile_commands.json")
if(NOT EXISTS "${compile_commands}")
  string(APPEND RunCMake_TEST_FAILED "${compile_commands} does not exist\n")
  return()
endif()
file(READ "${compile_commands}" json)
string(JSON n LENGTH "${json}")
math(EXPR last "${n} - 1")
set(found_other FALSE)
foreach(i RANGE ${last})
  string(JSON file GET "${json}" ${i} file)
  if(NOT file MATCHES "other\\.cpp$")
    continue()
  endif()
  set(found_other TRUE)
  # Makefile and Ninja write a command string. FASTBuild writes an
  # arguments array instead.
  string(JSON arguments_type ERROR_VARIABLE no_arguments
    TYPE "${json}" ${i} arguments)
  if(arguments_type STREQUAL "ARRAY")
    set(command)
    string(JSON argument_count LENGTH "${json}" ${i} arguments)
    if(argument_count GREATER 0)
      math(EXPR last_argument "${argument_count} - 1")
      foreach(j RANGE 0 ${last_argument})
        string(JSON argument GET "${json}" ${i} arguments ${j})
        string(APPEND command " ${argument}")
      endforeach()
    endif()
  else()
    string(JSON command GET "${json}" ${i} command)
  endif()
  check_flag("other.cpp" "${command}")
endforeach()
if(NOT found_other)
  string(APPEND RunCMake_TEST_FAILED
    "other.cpp not found in ${compile_commands}\n")
endif()
