cmake_minimum_required (VERSION 2.8)
project(CTestTestTimeout)
include(CTest)

if(NOT TIMEOUT)
  if(CYGWIN)
    set(TIMEOUT 4) # Cygwin CMake sometimes takes > 1 second to load!
  else()
    set(TIMEOUT 1)
  endif()
endif()

add_definitions(-DTIMEOUT=${TIMEOUT})
add_executable (Sleep sleep.c)

add_test(NAME TestTimeout
  COMMAND ${CMAKE_COMMAND} -D Sleep=$<TARGET_FILE:Sleep>
                           -D Log=${CMAKE_CURRENT_BINARY_DIR}/timeout.log
                           -P ${CMAKE_CURRENT_SOURCE_DIR}/timeout.cmake
  )
set_tests_properties(TestTimeout PROPERTIES TIMEOUT ${TIMEOUT})

add_test(NAME TestSleep COMMAND Sleep)
set_tests_properties(TestSleep PROPERTIES DEPENDS TestTimeout)
