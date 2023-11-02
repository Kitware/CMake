include(RunCMake)

run_cmake(InvalidArgument1)
run_cmake(exists)
if(NOT MSYS)
  # permissions and symbolic links are broken on MSYS
  unset(uid)
  unset(status)
  if(UNIX)
    set(ID "id")
    if (CMAKE_SYSTEM_NAME STREQUAL "SunOS" AND EXISTS "/usr/xpg4/bin/id")
      set (ID "/usr/xpg4/bin/id")
    endif()
    # if real user is root, tests are irrelevant
    execute_process(COMMAND ${ID} -u $ENV{USER} OUTPUT_VARIABLE uid ERROR_QUIET
                    RESULT_VARIABLE status OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif()
  if(NOT status AND NOT uid STREQUAL "0")
    run_cmake(FilePermissions)
  endif()
endif()
run_cmake(IsDirectory)
run_cmake(IsDirectoryLong)
run_cmake(duplicate-deep-else)
run_cmake(duplicate-else)
run_cmake(duplicate-else-after-elseif)
run_cmake(elseif-message)
run_cmake(misplaced-elseif)

run_cmake(unbalanced-parenthesis)

run_cmake(MatchesSelf)
run_cmake(IncompleteMatches)
run_cmake(IncompleteMatchesFail)

run_cmake(TestNameThatExists)
run_cmake(TestNameThatDoesNotExist)

run_cmake_script(AndOr)
