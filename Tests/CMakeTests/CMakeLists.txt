SET(CMAKE_EXECUTABLE "${EXECUTABLE_OUTPUT_PATH}/cmake")


MACRO(AddCMakeTest TestName PreArgs)
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/${TestName}Test.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${TestName}Test.cmake" @ONLY IMMEDIATE)
  ADD_TEST(CMake.${TestName} ${CMAKE_EXECUTABLE} ${PreArgs}
    -P "${CMAKE_CURRENT_BINARY_DIR}/${TestName}Test.cmake" ${ARGN})
ENDMACRO(AddCMakeTest)


AddCMakeTest(List "")
AddCMakeTest(VariableWatch "")
AddCMakeTest(Include "")
AddCMakeTest(FindBase "")
AddCMakeTest(Toolchain "")
AddCMakeTest(GetFilenameComponentRealpath "")
AddCMakeTest(Version "")
AddCMakeTest(Message "")
AddCMakeTest(File "")

SET(GetPrerequisites_PreArgs
  "-DCTEST_CONFIGURATION_TYPE:STRING=\\\${CTEST_CONFIGURATION_TYPE}"
  )
AddCMakeTest(GetPrerequisites "${GetPrerequisites_PreArgs}")
