PROJECT(TryCompile)

# try to compile a file that should compile
TRY_COMPILE(SHOULD_PASS
    ${TryCompile_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp  
    ${TryCompile_SOURCE_DIR}/pass.c
    OUTPUT_VARIABLE TRY_OUT)
IF(NOT SHOULD_PASS)
  MESSAGE(SEND_ERROR "should pass failed ${TRY_OUT}")
ENDIF(NOT SHOULD_PASS)

# try to compile a file that should not compile
TRY_COMPILE(SHOULD_FAIL
    ${TryCompile_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp  
    ${TryCompile_SOURCE_DIR}/fail.c
    OUTPUT_VARIABLE TRY_OUT)
IF(SHOULD_FAIL)
   MESSAGE(SEND_ERROR "Should fail passed ${TRY_OUT}")
ENDIF(SHOULD_FAIL)

# try to compile a file that should compile
TRY_COMPILE(SHOULD_PASS
    ${TryCompile_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp  
    ${TryCompile_SOURCE_DIR}/pass.c
    OUTPUT_VARIABLE TRY_OUT)
IF(NOT SHOULD_PASS)
  MESSAGE(SEND_ERROR "should pass failed ${TRY_OUT}")
ENDIF(NOT SHOULD_PASS)

# try to compile a file that should not compile
TRY_COMPILE(SHOULD_FAIL
    ${TryCompile_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp 
    ${TryCompile_SOURCE_DIR}/fail.c
    OUTPUT_VARIABLE TRY_OUT)
IF(SHOULD_FAIL)
   MESSAGE(SEND_ERROR "Should fail passed ${TRY_OUT}")
ENDIF(SHOULD_FAIL)

IF(NOT SHOULD_FAIL)
  IF(SHOULD_PASS)
    MESSAGE("All Tests passed, ignore all previous output.")
  ELSE(SHOULD_PASS)
    MESSAGE("Test failed")
  ENDIF(SHOULD_PASS)
ELSE(NOT SHOULD_FAIL)
  MESSAGE("Test failed")
ENDIF(NOT SHOULD_FAIL)
TRY_COMPILE(CMAKE_ANSI_FOR_SCOPE   
  ${TryCompile_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp 
    ${CMAKE_ROOT}/Modules/TestForAnsiForScope.cxx OUTPUT_VARIABLE OUT)
IF (CMAKE_ANSI_FOR_SCOPE)
   MESSAGE("Compiler supports ansi for")
ELSE(CMAKE_ANSI_FOR_SCOPE)
   MESSAGE("Compiler does not support ansi for scope")
ENDIF(CMAKE_ANSI_FOR_SCOPE)

TRY_COMPILE(CMAKE_ANSI_FOR_SCOPE
  ${TryCompile_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp 
    ${CMAKE_ROOT}/Modules/TestForAnsiForScope.cxx OUTPUT_VARIABLE OUT)
IF (CMAKE_ANSI_FOR_SCOPE)
   MESSAGE("Compiler supports ansi for")
ELSE(CMAKE_ANSI_FOR_SCOPE)
   MESSAGE("Compiler does not support ansi for scope")
ENDIF(CMAKE_ANSI_FOR_SCOPE)        

MESSAGE("use the module now")
INCLUDE(${CMAKE_ROOT}/Modules/TestForANSIForScope.cmake)
IF (CMAKE_ANSI_FOR_SCOPE)
   MESSAGE("Compiler supports ansi for")
ELSE(CMAKE_ANSI_FOR_SCOPE)
   MESSAGE("Compiler does not support ansi for scope")
ENDIF(CMAKE_ANSI_FOR_SCOPE)

ADD_EXECUTABLE(TryCompile pass.c)

######################################

# now two tests for TRY_RUN

# try to run a file that should compile and run without error
TRY_RUN(SHOULD_RUN SHOULD_COMPILE
    ${TryCompile_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp  
    ${TryCompile_SOURCE_DIR}/exit_success.c
    OUTPUT_VARIABLE TRY_OUT)
IF(NOT SHOULD_COMPILE)
  MESSAGE(SEND_ERROR "exit_success failed compiling: ${TRY_OUT}")
ENDIF(NOT SHOULD_COMPILE)
IF(NOT "${SHOULD_RUN}" STREQUAL "0")
  MESSAGE(SEND_ERROR "exit_success failed running with exit code ${SHOULD_RUN}")
ENDIF(NOT "${SHOULD_RUN}" STREQUAL "0")

# try to run a file that should compile and run, but return an error
TRY_RUN(SHOULD_EXIT_WITH_ERROR SHOULD_COMPILE
    ${TryCompile_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp  
    ${TryCompile_SOURCE_DIR}/exit_with_error.c
    OUTPUT_VARIABLE TRY_OUT)
IF(NOT SHOULD_COMPILE)
  MESSAGE(STATUS " exit_with_error failed compiling: ${TRY_OUT}")
ENDIF(NOT SHOULD_COMPILE)
IF("${SHOULD_EXIT_WITH_ERROR}" STREQUAL "0")
  MESSAGE(SEND_ERROR " exit_with_error passed with exit code ${SHOULD_EXIT_WITH_ERROR}")
ENDIF("${SHOULD_EXIT_WITH_ERROR}" STREQUAL "0")

