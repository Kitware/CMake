find_package(Git QUIET REQUIRED)

include(ExternalProject)

set(srcRepo ${CMAKE_CURRENT_BINARY_DIR}/srcRepo)
set(srcDir  ${CMAKE_CURRENT_BINARY_DIR}/src)
set(binDir  ${CMAKE_CURRENT_BINARY_DIR}/build)
file(MAKE_DIRECTORY ${srcRepo})
file(MAKE_DIRECTORY ${srcDir})

file(GLOB entries ${srcRepo}/*)
file(REMOVE_RECURSE ${entries} ${binDir})
file(TOUCH ${srcRepo}/firstFile.txt)
configure_file(${CMAKE_CURRENT_LIST_DIR}/FetchGitTags/CMakeLists.txt
    ${srcDir}/CMakeLists.txt COPYONLY)

function(execGitCommand)
  execute_process(
    WORKING_DIRECTORY ${srcRepo}
    COMMAND ${GIT_EXECUTABLE} ${ARGN}
    COMMAND_ECHO STDOUT
    COMMAND_ERROR_IS_FATAL ANY
  )
endfunction()

function(configureAndBuild tag)
  execute_process(COMMAND ${CMAKE_COMMAND}
      -G ${CMAKE_GENERATOR} -T "${CMAKE_GENERATOR_TOOLSET}"
      -A "${CMAKE_GENERATOR_PLATFORM}"
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
    COMMAND_ERROR_IS_FATAL ANY
  )
endfunction()

# Setup a fresh source repo with a predictable default branch across all
# git versions
execGitCommand(-c init.defaultBranch=master init)
execGitCommand(config --add user.email "testauthor@cmake.org")
execGitCommand(config --add user.name testauthor)

# Create the initial repo structure
execGitCommand(add firstFile.txt)
execGitCommand(commit -m "First file")

message(STATUS "First configure-and-build")
configureAndBuild(master)

# Create a tagged commit that is not on any branch. With git 2.20 or later,
# this commit won't be fetched without the --tags option.
file(TOUCH ${srcRepo}/secondFile.txt)
execGitCommand(add secondFile.txt)
execGitCommand(commit -m "Second file")
execGitCommand(tag -a -m "Adding tag" tag_of_interest)
execGitCommand(reset --hard HEAD~1)

message(STATUS "Second configure-and-build")
configureAndBuild(tag_of_interest)
