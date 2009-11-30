cmake_minimum_required (VERSION 2.8)
PROJECT(CTestTestTimeout)

SET(DART_ROOT "" CACHE STRING "" FORCE)
ENABLE_TESTING()
INCLUDE (${CMAKE_ROOT}/Modules/Dart.cmake)

GET_FILENAME_COMPONENT(CTEST_COMMAND "${CMAKE_COMMAND}" PATH)
SET(CTEST_COMMAND "${CTEST_COMMAND}/ctest")

ADD_EXECUTABLE (Timeout timeout.c)

ENABLE_TESTING ()

ADD_TEST(NAME TestTimeout
  COMMAND ${CMAKE_COMMAND} -D Timeout=$<TARGET_FILE:Timeout>
                           -D Log=${CMAKE_CURRENT_BINARY_DIR}/timeout.log
                           -P ${CMAKE_CURRENT_SOURCE_DIR}/timeout.cmake
  )
SET_TESTS_PROPERTIES(TestTimeout PROPERTIES TIMEOUT 1)

ADD_TEST(NAME CheckChild
  COMMAND ${CMAKE_COMMAND} -D Timeout=$<TARGET_FILE:Timeout>
                           -D Log=${CMAKE_CURRENT_BINARY_DIR}/timeout.log
                           -P ${CMAKE_CURRENT_SOURCE_DIR}/check.cmake
  )
SET_TESTS_PROPERTIES(CheckChild PROPERTIES DEPENDS TestTimeout)


INCLUDE (CTest)
