# determine the compiler to use for C programs
# NOTE, a generator may set CMAKE_C_COMPILER before
# loading this file to force a compiler.

FIND_PROGRAM(CMAKE_C_COMPILER_FULLPATH NAMES $ENV{CC} gcc cc cl bcc )
GET_FILENAME_COMPONENT(CMAKE_C_COMPILER ${CMAKE_C_COMPILER_FULLPATH} NAME_WE)
FIND_PROGRAM(CMAKE_AR NAMES ar PATHS /bin /usr/bin /usr/local/bin)

FIND_PROGRAM(CMAKE_RANLIB NAMES ranlib PATHS /bin /usr/bin /usr/local/bin)
IF(NOT CMAKE_RANLIB)
   SET(RANLIB :)
ENDIF(NOT CMAKE_RANLIB)

# test to see if the c compiler is gnu
EXEC_PROGRAM(${CMAKE_C_COMPILER} ARGS -E ${CMAKE_ROOT}/Modules/CMakeTestGNU.c OUTPUT_VARIABLE CMAKE_COMPILER_OUTPUT RETURN_VALUE CMAKE_COMPILER_RETURN)
IF(NOT CMAKE_COMPILER_RETURN)
   IF(${CMAKE_COMPILER_OUTPUT} MATCHES ".*THIS_IS_GNU.*" )
      SET(CMAKE_COMPILER_IS_GNUGCC 1)
   ENDIF(${CMAKE_COMPILER_OUTPUT} MATCHES ".*THIS_IS_GNU.*" )
ENDIF(NOT CMAKE_COMPILER_RETURN)

# configure variables set in this file for fast reload later on
CONFIGURE_FILE(${CMAKE_ROOT}/Modules/CMakeCCompiler.cmake.in 
               ${PROJECT_BINARY_DIR}/CMakeCCompiler.cmake)
