SET(CMAKE_EXECUTABLE "${EXECUTABLE_OUTPUT_PATH}/cmake")


MACRO(AddCMakeTest TestName Arguments)
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/${TestName}Test.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${TestName}Test.cmake" @ONLY IMMEDIATE)
  ADD_TEST(CMake.${TestName} ${CMAKE_EXECUTABLE} -P "${CMAKE_CURRENT_BINARY_DIR}/${TestName}Test.cmake" ${Arguments})
ENDMACRO(AddCMakeTest)

AddCMakeTest(List "")
AddCMakeTest(VariableWatch "")
AddCMakeTest(Include "")
AddCMakeTest(FindBase "")
AddCMakeTest(Toolchain "")
