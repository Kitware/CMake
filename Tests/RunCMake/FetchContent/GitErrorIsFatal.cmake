find_package(Git QUIET REQUIRED)
include(FetchContent)

set(srcRepo ${CMAKE_CURRENT_BINARY_DIR}/srcRepo)
file(REMOVE_RECURSE ${srcRepo})
file(MAKE_DIRECTORY ${srcRepo})

function(execGitCommand)
  execute_process(
    WORKING_DIRECTORY ${srcRepo}
    COMMAND ${GIT_EXECUTABLE} ${ARGN}
    OUTPUT_QUIET
    COMMAND_ERROR_IS_FATAL ANY
  )
endfunction()

function(commitFile name out_var)
  file(TOUCH ${srcRepo}/${name})
  execGitCommand(add ${name})
  execGitCommand(commit -m "Add ${name}")
  execute_process(
    WORKING_DIRECTORY ${srcRepo}
    COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
    OUTPUT_VARIABLE commit_hash
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
  )
  set(${out_var} ${commit_hash} PARENT_SCOPE)
endfunction()

# Populates the repository directly at the given commit, and checks that
# it was checked out.
function(populateAt commit)
  FetchContent_Populate(pinned
    GIT_REPOSITORY ${srcRepo}
    GIT_TAG ${commit}
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/pinned-src
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/pinned-build
    SUBBUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/pinned-subbuild
  )
  execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/pinned-src
    COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
    OUTPUT_VARIABLE head
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
  )
  if(NOT head STREQUAL commit)
    message(FATAL_ERROR "Checked out ${head}, expected ${commit}")
  endif()
endfunction()

execGitCommand(-c init.defaultBranch=master init)
execGitCommand(config --add user.email "testauthor@cmake.org")
execGitCommand(config --add user.name testauthor)
execGitCommand(config commit.gpgsign "false")
commitFile(firstFile.txt firstCommit)
commitFile(secondFile.txt secondCommit)

# Direct population runs the steps in this scope. Git commands there that
# are expected to fail, such as looking up a commit hash as a ref, or the
# branch of a detached HEAD when updating to another commit, must not stop
# them when errors are made fatal.
set(CMAKE_EXECUTE_PROCESS_COMMAND_ERROR_IS_FATAL ANY)
populateAt(${firstCommit})
# With timestamps of one second resolution, the second update script could
# otherwise be no newer than the first update's stamp, and not run.
execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1.125)
populateAt(${secondCommit})
unset(CMAKE_EXECUTE_PROCESS_COMMAND_ERROR_IS_FATAL)
