find_package(Git QUIET REQUIRED)

include(ExternalProject)

set(srcRepo ${CMAKE_CURRENT_BINARY_DIR}/srcRepo)
set(srcDir  ${CMAKE_CURRENT_BINARY_DIR}/src)
set(binDir  ${CMAKE_CURRENT_BINARY_DIR}/build)
file(REMOVE_RECURSE ${srcRepo} ${binDir})
file(MAKE_DIRECTORY ${srcRepo})
file(MAKE_DIRECTORY ${srcDir})
file(TOUCH ${srcRepo}/firstFile.txt)
configure_file(${CMAKE_CURRENT_LIST_DIR}/FetchGitRefs/CMakeLists.txt
    ${srcDir}/CMakeLists.txt COPYONLY)

function(execGitCommand)
  execute_process(
    WORKING_DIRECTORY ${srcRepo}
    COMMAND ${GIT_EXECUTABLE} ${ARGN}
    COMMAND_ECHO STDOUT
    COMMAND_ERROR_IS_FATAL ANY
  )
endfunction()

set(generatorArgs "")
if(CMAKE_GENERATOR_INSTANCE)
  list(APPEND generatorArgs -D "CMAKE_GENERATOR_INSTANCE=${CMAKE_GENERATOR_INSTANCE}")
endif()
if(CMAKE_MAKE_PROGRAM)
  list(APPEND generatorArgs -D "CMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}")
endif()

# Setup a fresh source repo with a predictable default branch across all
# git versions
execGitCommand(-c init.defaultBranch=master init)
execGitCommand(config --add user.email "testauthor@cmake.org")
execGitCommand(config --add user.name testauthor)
execGitCommand(config commit.gpgsign "false")
execGitCommand(add firstFile.txt)
execGitCommand(commit -m "First file")

# Git names the remote of a clone after clone.defaultRemoteName, but the
# update step fetches a branch from the remote named by GIT_REMOTE_NAME,
# "origin" by default, so the clone must name it that.
set(ENV{GIT_CONFIG_COUNT} 1)
set(ENV{GIT_CONFIG_KEY_0} clone.defaultRemoteName)
set(ENV{GIT_CONFIG_VALUE_0} upstream)
message(STATUS "Configure-and-build at master with another default remote name")
execute_process(COMMAND ${CMAKE_COMMAND}
    -G ${CMAKE_GENERATOR} -T "${CMAKE_GENERATOR_TOOLSET}"
    -A "${CMAKE_GENERATOR_PLATFORM}"
    ${generatorArgs}
    -D repoDir:PATH=${srcRepo}
    -D gitTag:STRING=master
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
unset(ENV{GIT_CONFIG_COUNT})
unset(ENV{GIT_CONFIG_KEY_0})
unset(ENV{GIT_CONFIG_VALUE_0})

# Look only at the clone's own configuration: the user's may add remotes.
execute_process(
  WORKING_DIRECTORY ${binDir}/fetcher-prefix/src/fetcher
  COMMAND ${GIT_EXECUTABLE} config --local --get remote.origin.url
  RESULT_VARIABLE result
  OUTPUT_QUIET
)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "The clone has no remote named origin")
endif()
