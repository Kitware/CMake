include(RunCMake)

run_cmake(InvalidArgument1)
run_cmake(exists)
if(NOT MSYS)
  # permissions and symbolic links are broken on MSYS
  # if real user is root, tests are irrelevant
  get_unix_uid(uid)
  if(NOT uid STREQUAL "0")
    run_cmake_with_options(FilePermissions -DNO_WRITABLE_DIR=${CMake_TEST_NO_WRITE_ONLY_DIR})
  endif()
endif()
run_cmake(IsDirectory)
run_cmake(IsDirectoryLong)
run_cmake(duplicate-deep-else)
run_cmake(duplicate-else)
run_cmake(duplicate-else-after-elseif)
run_cmake(elseif-message)
run_cmake(empty-elseif-warning)
run_cmake(misplaced-elseif)

run_cmake(unbalanced-parenthesis)

run_cmake(MatchesSelf)
run_cmake(IncompleteMatches)
run_cmake(IncompleteMatchesFail)

run_cmake(TestNameThatExists)
run_cmake(TestNameThatDoesNotExist)

run_cmake_script(AndOr)
