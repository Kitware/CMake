set(CVSROOT ":pserver:anonymous@www.cmake.org:/cvsroot/CMake")
get_filename_component(SCRIPT_PATH "${CMAKE_CURRENT_LIST_FILE}" PATH)

if(NOT DEFINED INSTALLER_SUFFIX)
  set(INSTALLER_SUFFIX "*.sh")
endif(NOT DEFINED INSTALLER_SUFFIX)
if(NOT DEFINED PROCESSORS)
  set(PROCESSORS 1)
endif(NOT DEFINED PROCESSORS)
if(NOT DEFINED CMAKE_VERSION)
  message(FATAL_ERROR "CMAKE_VERSION not defined")
endif(NOT DEFINED CMAKE_VERSION)
if(NOT DEFINED CVS_COMMAND)
  set(CVS_COMMAND cvs)
endif(NOT DEFINED CVS_COMMAND)

if("${CMAKE_VERSION}" STREQUAL "CVS")
  set( CMAKE_CHECKOUT "${CVS_COMMAND} -q -z3 -d ${CVSROOT} export -D now ")
  set( CMAKE_VERSION "CurrentCVS")
else("${CMAKE_VERSION}" STREQUAL "CVS")
  set( CMAKE_CHECKOUT "${CVS_COMMAND} -q -z3 -d ${CVSROOT} export -r ${CMAKE_VERSION} ")
endif("${CMAKE_VERSION}" STREQUAL "CVS")

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
    execute_process(COMMAND ssh ${HOST} ${EXTRA_HOP} ${command} RESULT_VARIABLE result INPUT_FILE ${ARGV2})
  else(${ARGC} GREATER 2)
    execute_process(COMMAND ssh ${HOST} ${EXTRA_HOP} ${command} RESULT_VARIABLE result) 
  endif(${ARGC} GREATER 2)
  if(${result} GREATER 0)
    message(FATAL_ERROR "Error running command: ${command}, return value = ${result}")
  endif(${result} GREATER 0)
endmacro(remote_command)

set(CMAKE_BACKWARDS_COMPATIBILITY 2.4)
configure_file(${SCRIPT_PATH}/release_cmake.sh.in release_cmake-${HOST}.sh)
file(READ release_cmake.sh RELEASE_CMAKE_CONTENTS)
remote_command("Copy release_cmake.sh to sever"
  "echo '${RELEASE_CMAKE_CONTENTS}' > release_cmake-${HOST}.sh")

remote_command("Run release script" "sh release_cmake-${HOST}.sh")

message("copy the .gz file back from the machine")
execute_process(COMMAND scp ${HOST}:CMakeReleaseDirectory/${CMAKE_VERSION}-build/*.gz .
  RESULT_VARIABLE result) 

message("copy the ${INSTALLER_SUFFIX} file back from the machine")
execute_process(COMMAND scp ${HOST}:CMakeReleaseDirectory/${CMAKE_VERSION}-build/${INSTALLER_SUFFIX} .
  RESULT_VARIABLE result) 

