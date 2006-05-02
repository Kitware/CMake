set(CVSROOT ":pserver:anonymous@www.cmake.org:/cvsroot/CMake")
# used for -j builds
if(NOT DEFINED PROCESSORS)
  set(PROCESSORS 1)
endif(NOT DEFINED PROCESSORS)
if(NOT DEFINED CMAKE_VERSION)
  message(FATAL_ERROR "CMAKE_VERSION not defined")
endif(NOT DEFINED CMAKE_VERSION)
if(NOT HOST)
  message(FATAL_ERROR "HOST must be specified with -DHOST=host")
endif(NOT HOST)
if(NOT DEFINED MAKE)
  message(FATAL_ERROR "MAKE must be specified with -DMAKE=\"make -j2\"")
endif(NOT DEFINED MAKE)
  
message("Creating CMake release ${CMAKE_VERSION} on ${HOST} with parallel = ${PROCESSORS}")

# define a macro to run a remote command
macro(remote_command comment command)
  message("${comment}")
  if(${ARGC} GREATER 2)
    execute_process(COMMAND ssh ${HOST} ${command} RESULT_VARIABLE result INPUT_FILE ${ARGV2})
  else(${ARGC} GREATER 2)
    execute_process(COMMAND ssh ${HOST} ${command} RESULT_VARIABLE result) 
  endif(${ARGC} GREATER 2)
  message("${result}")
  if(${result} GREATER 0)
    message(FATAL_ERROR "Error running command: ${command}, return value = ${result}")
  endif(${result} GREATER 0)
endmacro(remote_command)

# remove and create a directory to work with
remote_command(
  "remove and create working directory ~/CMakeReleaseDirectory"
  "rm -rf ~/CMakeReleaseDirectory && mkdir ~/CMakeReleaseDirectory")
# login to cvs
remote_command(
  "Login into cvs."
  "cvs -d ${CVSROOT} login" cmake_login)
message(FATAL_ERROR "done")
# checkout the source
remote_command(
  "Checkout the source for ${CMAKE_VERSION}"
  "cd ~/CMakeReleaseDirectory && cvs -q -z3 -d ${CVSROOT} export -r ${CMAKE_VERSION} -d ${CMAKE_VERSION} CMake")
# create the build directory
remote_command(
  "Create a directory to build in"
  "rm -rf ~/CMakeReleaseDirectory/${CMAKE_VERSION}-build && mkdir ~/CMakeReleaseDirectory/${CMAKE_VERSION}-build")
# now bootstrap cmake
remote_command(
  "Run cmake bootstrap --parallel=${PROCESSORS}"
  "cd ~/CMakeReleaseDirectory/${CMAKE_VERSION}-build && ../${CMAKE_VERSION}/bootstrap --parallel=${PROCESSORS}")
# build cmake
remote_command(
  "Build cmake with ${MAKE}"
  "cd ~/CMakeReleaseDirectory/${CMAKE_VERSION}-build && ${MAKE}")
# build cmake
remote_command(
  "Build cmake with ${MAKE}"
  "cd ~/CMakeReleaseDirectory/${CMAKE_VERSION}-build && ${MAKE}")
# run the tests
remote_command(
  "Run cmake tests"
  "cd ~/CMakeReleaseDirectory/${CMAKE_VERSION}-build && ${MAKE} test")
# package cmake with self-extracting shell script
remote_command(
  "Package cmake"
  "cd ~/CMakeReleaseDirectory/${CMAKE_VERSION}-build && ${MAKE} package")
# package cmake with a tar gz file
remote_command(
  "Package cmake"
  "cd ~/CMakeReleaseDirectory/${CMAKE_VERSION}-build && ./bin/cpack -G TGZ")

message("copy the .gz file back from the machine")
execute_process(COMMAND scp ${HOST}:CMakeReleaseDirectory/${CMAKE_VERSION}-build/*.gz .
  RESULT_VARIABLE result) 
message("copy the .sh file back from the machine")
execute_process(COMMAND scp ${HOST}:CMakeReleaseDirectory/${CMAKE_VERSION}-build/*.sh .
  RESULT_VARIABLE result) 
