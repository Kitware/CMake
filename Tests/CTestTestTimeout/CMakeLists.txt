cmake_minimum_required (VERSION 2.8)
PROJECT(CTestTestTimeout)
INCLUDE(CTest)

IF(NOT TIMEOUT)
  IF(CYGWIN)
    SET(TIMEOUT 4) # Cygwin CMake sometimes takes > 1 second to load!
  ELSE()
    SET(TIMEOUT 1)
  ENDIF()
ENDIF()

ADD_DEFINITIONS(-DTIMEOUT=${TIMEOUT})
ADD_EXECUTABLE (Timeout timeout.c)

ADD_TEST(NAME TestTimeout
  COMMAND ${CMAKE_COMMAND} -D Timeout=$<TARGET_FILE:Timeout>
                           -D Log=${CMAKE_CURRENT_BINARY_DIR}/timeout.log
                           -P ${CMAKE_CURRENT_SOURCE_DIR}/timeout.cmake
  )
SET_TESTS_PROPERTIES(TestTimeout PROPERTIES TIMEOUT ${TIMEOUT})

ADD_TEST(NAME CheckChild
  COMMAND ${CMAKE_COMMAND} -D Timeout=$<TARGET_FILE:Timeout>
                           -D Log=${CMAKE_CURRENT_BINARY_DIR}/timeout.log
                           -P ${CMAKE_CURRENT_SOURCE_DIR}/check.cmake
  )
SET_TESTS_PROPERTIES(CheckChild PROPERTIES DEPENDS TestTimeout)
