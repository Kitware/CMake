SET(CMAKE_EXECUTABLE "${CMake_BIN_DIR}/cmake")


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
AddCMakeTest(ConfigureFile "")
AddCMakeTest(SeparateArguments "")
AddCMakeTest(ImplicitLinkInfo "")
AddCMakeTest(ModuleNotices "")
AddCMakeTest(If "")
AddCMakeTest(String "")
AddCMakeTest(Math "")
AddCMakeTest(CMakeMinimumRequired "")
AddCMakeTest(CompilerIdVendor "")

AddCMakeTest(FileDownload "")
set_property(TEST CMake.FileDownload PROPERTY
  PASS_REGULAR_EXPRESSION "file already exists with expected MD5 sum"
  )

if(HAVE_ELF_H)
  AddCMakeTest(ELF "")
endif()

SET(EndStuff_PreArgs
  "-Ddir:STRING=${CMAKE_CURRENT_BINARY_DIR}/EndStuffTest"
  )
AddCMakeTest(EndStuff "${EndStuff_PreArgs}")

SET(GetPrerequisites_PreArgs
  "-DCTEST_CONFIGURATION_TYPE:STRING=\\\${CTEST_CONFIGURATION_TYPE}"
  )
AddCMakeTest(GetPrerequisites "${GetPrerequisites_PreArgs}")

# Run CheckSourceTree as the very last test in the CMake/CTest/CPack test
# suite. It detects if any changes have been made to the CMake source tree
# by any previous configure, build or test steps.
#
if(do_cvs_tests OR GIT_EXECUTABLE)
  string(REPLACE "\\" "/" ENV_HOME "$ENV{HOME}")
  set(CheckSourceTree_PreArgs
    "-DCMake_BINARY_DIR:PATH=${CMake_BINARY_DIR}"
    "-DCMake_SOURCE_DIR:PATH=${CMake_SOURCE_DIR}"
    "-DCVS_EXECUTABLE:STRING=${CVS_EXECUTABLE}"
    "-DGIT_EXECUTABLE:STRING=${GIT_EXECUTABLE}"
    "-DHOME:STRING=${ENV_HOME}"
    )
  AddCMakeTest(CheckSourceTree "${CheckSourceTree_PreArgs}")
endif(do_cvs_tests OR GIT_EXECUTABLE)
