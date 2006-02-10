SET(CMAKE_EXECUTABLE "${EXECUTABLE_OUTPUT_PATH}/cmake")


MACRO(AddCMakeTest TestName Arguments)
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/${TestName}Test.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${TestName}Test.cmake" @ONLY IMMEDIATE)
  ADD_TEST(CMake.List ${CMAKE_EXECUTABLE} -P "${CMAKE_CURRENT_BINARY_DIR}/ListTest.cmake" ${Arguments})
ENDMACRO(AddCMakeTest)

AddCMakeTest(List "")
