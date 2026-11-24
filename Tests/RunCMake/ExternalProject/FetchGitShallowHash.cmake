find_package(Git QUIET REQUIRED)

include(ExternalProject)

set(srcRepo ${CMAKE_CURRENT_BINARY_DIR}/srcRepo)
set(srcDir  ${CMAKE_CURRENT_BINARY_DIR}/src)
set(binDir  ${CMAKE_CURRENT_BINARY_DIR}/build)
set(fetchedDir ${binDir}/fetcher-prefix/src/fetcher)
file(REMOVE_RECURSE ${srcRepo} ${binDir})
file(MAKE_DIRECTORY ${srcRepo})
file(MAKE_DIRECTORY ${srcDir})
configure_file(${CMAKE_CURRENT_LIST_DIR}/FetchGitShallowHash/CMakeLists.txt
    ${srcDir}/CMakeLists.txt COPYONLY)

# Use a file:// URL. git ignores --depth when cloning a plain local path.
set(repoUrl "file://${srcRepo}")

function(execGitCommand)
  execute_process(
    WORKING_DIRECTORY ${srcRepo}
    COMMAND ${GIT_EXECUTABLE} ${ARGN}
    COMMAND_ECHO STDOUT
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
    COMMAND_ERROR_IS_FATAL ANY
    OUTPUT_VARIABLE commit_hash
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(${out_var} ${commit_hash} PARENT_SCOPE)
endfunction()

function(configureAndBuild tag)
  execute_process(COMMAND ${CMAKE_COMMAND}
      -G ${CMAKE_GENERATOR} -T "${CMAKE_GENERATOR_TOOLSET}"
      -A "${CMAKE_GENERATOR_PLATFORM}"
      -D repoUrl:STRING=${repoUrl}
      -D gitTag:STRING=${tag}
      -B ${binDir}
      -S ${srcDir}
    COMMAND_ECHO STDOUT
    COMMAND_ERROR_IS_FATAL ANY
  )

  execute_process(COMMAND ${CMAKE_COMMAND} --build ${binDir} --target fetcher
    WORKING_DIRECTORY ${binDir}
    COMMAND_ECHO STDOUT
    COMMAND_ERROR_IS_FATAL ANY
  )
endfunction()

function(checkFetched)
  if(NOT EXISTS ${fetchedDir}/.git/shallow)
    message(FATAL_ERROR "The clone in ${fetchedDir} is not shallow")
  endif()
  foreach(name IN LISTS ARGN)
    if(NOT EXISTS ${fetchedDir}/${name})
      message(FATAL_ERROR "${name} was not checked out in ${fetchedDir}")
    endif()
  endforeach()
endfunction()

function(checkNotFetched name)
  if(EXISTS ${fetchedDir}/${name})
    message(FATAL_ERROR "${name} should not be checked out in ${fetchedDir}")
  endif()
endfunction()

# Setup a fresh source repo with a predictable default branch across all
# git versions. Let it send commits that are not branch tips or tags, as
# git servers commonly do, even to clients using git protocol version 0.
execGitCommand(-c init.defaultBranch=master init)
execGitCommand(config --add user.email "testauthor@cmake.org")
execGitCommand(config --add user.name testauthor)
execGitCommand(config commit.gpgsign "false")
execGitCommand(config uploadpack.allowReachableSHA1InWant "true")

# Three commits on one branch. Only the last is a branch tip, and none is
# tagged, so a shallow clone of the branch tips contains neither of the
# first two.
commitFile(firstFile.txt firstCommit)
commitFile(secondFile.txt secondCommit)
commitFile(thirdFile.txt thirdCommit)

message(STATUS "Configure-and-build, clone at commit hash ${secondCommit}")
configureAndBuild(${secondCommit})
checkFetched(firstFile.txt secondFile.txt)
checkNotFetched(thirdFile.txt)

message(STATUS "Configure-and-build, update to commit hash ${firstCommit}")
configureAndBuild(${firstCommit})
checkFetched(firstFile.txt)
checkNotFetched(secondFile.txt)
