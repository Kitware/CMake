cmake_minimum_required(VERSION 3.25)
project(${RunCMake_TEST} NONE)

find_package(Git REQUIRED)

function(execGitCommand workDir)
  execute_process(
    WORKING_DIRECTORY "${workDir}"
    COMMAND "${GIT_EXECUTABLE}" ${ARGN}
    COMMAND_ECHO STDOUT
    COMMAND_ERROR_IS_FATAL ANY
  )
endfunction()

function(initGitRepo workDir)
  # init.defaultBranch only works with git 2.28 or later, so we must use the
  # historical default branch name "master". Force the old default in case test
  # sites have overridden the default to something else.
  execGitCommand("${workDir}" -c init.defaultBranch=master init)
  execGitCommand("${workDir}" config user.email "testauthor@cmake.org")
  execGitCommand("${workDir}" config user.name testauthor)
  execGitCommand("${workDir}" config core.autocrlf false)
  execGitCommand("${workDir}" add CMakeLists.txt)
  execGitCommand("${workDir}" commit -m "Initial commit")
endfunction()

include(${RunCMake_TEST}.cmake)
