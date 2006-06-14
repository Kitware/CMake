PROJECT(TryCompile)

# try to compile a file that should compile
TRY_COMPILE(SHOULD_PASS
    ${TryCompile_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp  
    ${TryCompile_SOURCE_DIR}/pass.c
    OUTPUT_VARIABLE TRY_OUT)
IF(NOT SHOULD_PASS)
  MESSAGE(SEND_ERROR "should pass failed ")
ENDIF(NOT SHOULD_PASS)

# try to compile a file that should not compile
TRY_COMPILE(SHOULD_FAIL
    ${TryCompile_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp  
    ${TryCompile_SOURCE_DIR}/fail.c
    OUTPUT_VARIABLE TRY_OUT)
IF(SHOULD_FAIL)
   MESSAGE(SEND_ERROR "Should fail passed")
ENDIF(SHOULD_FAIL)

# try to compile a file that should compile
TRY_COMPILE(SHOULD_PASS
    ${TryCompile_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp  
    ${TryCompile_SOURCE_DIR}/pass.c
    OUTPUT_VARIABLE TRY_OUT)
IF(NOT SHOULD_PASS)
  MESSAGE(SEND_ERROR "should pass failed ")
ENDIF(NOT SHOULD_PASS)

# try to compile a file that should not compile
TRY_COMPILE(SHOULD_FAIL
    ${TryCompile_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp 
    ${TryCompile_SOURCE_DIR}/fail.c
    OUTPUT_VARIABLE TRY_OUT)
IF(SHOULD_FAIL)
   MESSAGE(SEND_ERROR "Should fail passed")
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
