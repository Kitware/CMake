PROJECT(TryCompileTest)

# try to compile a file that should compile
TRY_COMPILE(SHOULD_PASS
    ${TryCompileTest_BINARY_DIR}/CMakeTmp  
    ${TryCompileTest_SOURCE_DIR}/pass.c
    OUTPUT_VARIABLE TRY_OUT)
IF(NOT SHOULD_PASS)
  MESSAGE(SEND_ERROR "should pass failed ")
ENDIF(NOT SHOULD_PASS)
MESSAGE( "output from TRY_COMPILE ${TRY_OUT} ")

# try to compile a file that should not compile
TRY_COMPILE(SHOULD_FAIL
    ${TryCompileTest_BINARY_DIR}/CMakeTmp  
    ${TryCompileTest_SOURCE_DIR}/fail.c
    OUTPUT_VARIABLE TRY_OUT)
IF(SHOULD_FAIL)
   MESSAGE(SEND_ERROR "Should fail passed")
ENDIF(SHOULD_FAIL)
MESSAGE("output from TRY_COMPILE ${TRY_OUT} ")

# try to compile a file that should compile
TRY_COMPILE(SHOULD_PASS
    ${TryCompileTest_BINARY_DIR}/CMakeTmp  
    ${TryCompileTest_SOURCE_DIR}/pass.c
    OUTPUT_VARIABLE TRY_OUT)
IF(NOT SHOULD_PASS)
  MESSAGE(SEND_ERROR "should pass failed ")
ENDIF(NOT SHOULD_PASS)
MESSAGE("output from TRY_COMPILE ${TRY_OUT} ")

# try to compile a file that should not compile
TRY_COMPILE(SHOULD_FAIL
    ${TryCompileTest_BINARY_DIR}/CMakeTmp 
    ${TryCompileTest_SOURCE_DIR}/fail.c
    OUTPUT_VARIABLE TRY_OUT)
IF(SHOULD_FAIL)
   MESSAGE(SEND_ERROR "Should fail passed")
ENDIF(SHOULD_FAIL)
MESSAGE("output from TRY_COMPILE ${TRY_OUT} ")

IF(NOT SHOULD_FAIL)
  IF(SHOULD_PASS)
    MESSAGE("All Tests passed, ignore all previous output.")
  ELSE(SHOULD_PASS)
    MESSAGE("Test failed")
  ENDIF(SHOULD_PASS)
ELSE(NOT SHOULD_FAIL)
  MESSAGE("Test failed")
ENDIF(NOT SHOULD_FAIL)

 
