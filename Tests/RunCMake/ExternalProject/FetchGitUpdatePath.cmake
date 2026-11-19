find_package(Git QUIET REQUIRED)

include(ExternalProject)

set(srcRepo ${CMAKE_CURRENT_BINARY_DIR}/srcRepo)
set(srcDir  ${CMAKE_CURRENT_BINARY_DIR}/src)
set(binDir  ${CMAKE_CURRENT_BINARY_DIR}/build)
file(REMOVE_RECURSE ${srcRepo} ${binDir})
file(MAKE_DIRECTORY ${srcRepo}/docs)
file(MAKE_DIRECTORY ${srcDir})
file(TOUCH ${srcRepo}/docs/firstFile.txt)
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

# Configures and builds at the given tag, and sets build_result and
# build_output.
function(configureAndBuild tag)
  execute_process(COMMAND ${CMAKE_COMMAND}
      -G ${CMAKE_GENERATOR} -T "${CMAKE_GENERATOR_TOOLSET}"
      -A "${CMAKE_GENERATOR_PLATFORM}"
      ${generatorArgs}
      -D repoDir:PATH=${srcRepo}
      -D gitTag:STRING=${tag}
      -B ${binDir}
      -S ${srcDir}
    COMMAND_ECHO STDOUT
    COMMAND_ERROR_IS_FATAL ANY
  )

  execute_process(COMMAND ${CMAKE_COMMAND} --build ${binDir} --target fetcher
    WORKING_DIRECTORY ${binDir}
    COMMAND_ECHO STDOUT
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
  )
  set(build_result "${result}" PARENT_SCOPE)
  set(build_output "${output}" PARENT_SCOPE)
endfunction()

# Setup a fresh source repo with a predictable default branch across all
# git versions
execGitCommand(-c init.defaultBranch=master init)
execGitCommand(config --add user.email "testauthor@cmake.org")
execGitCommand(config --add user.name testauthor)
execGitCommand(config commit.gpgsign "false")
execGitCommand(add docs/firstFile.txt)
execGitCommand(commit -m "First file")

message(STATUS "Configure-and-build at master")
configureAndBuild(master)
if(NOT build_result EQUAL 0)
  message(FATAL_ERROR "Build failed:\n${build_output}")
endif()

# A tag that is neither a branch nor a tag, but names a path in the
# checkout, can't be checked out. The update step must not restore the
# files at that path instead, leaving the checkout where it was. Local
# changes it stashes are put back.
set(checkoutDir ${binDir}/fetcher-prefix/src/fetcher)
set(localFile ${checkoutDir}/docs/firstFile.txt)
file(WRITE ${localFile} "local change\n")
# Git before 2.21 can't stash the local change without a committer
# identity, and on some hosts it can't guess one.
execGitCommand(-C ${checkoutDir} config user.email "testauthor@cmake.org")
execGitCommand(-C ${checkoutDir} config user.name testauthor)
message(STATUS "Configure-and-build at docs, a path but not a ref")
configureAndBuild(docs)
if(build_result EQUAL 0)
  message(FATAL_ERROR "Build at a tag that only names a path succeeded:\n${build_output}")
endif()
if(NOT build_output MATCHES "Failed to checkout: 'docs'")
  message(FATAL_ERROR "Build did not fail to check out docs:\n${build_output}")
endif()
file(READ ${localFile} localContent)
if(NOT localContent STREQUAL "local change\n")
  message(FATAL_ERROR "The local change was not put back:\n${localContent}")
endif()
