set(CMAKE_EXECUTABLE "${CMake_BIN_DIR}/cmake")


macro(AddCMakeTest TestName PreArgs)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${TestName}Test.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${TestName}Test.cmake" @ONLY)
  add_test(NAME CMake.${TestName}
    COMMAND ${CMAKE_EXECUTABLE} ${PreArgs}
    -P "${CMAKE_CURRENT_BINARY_DIR}/${TestName}Test.cmake" ${ARGN})
  set_tests_properties("CMake.${TestName}" PROPERTIES LABELS "CMake;command")
endmacro()


AddCMakeTest(List "")
AddCMakeTest(VariableWatch "")
AddCMakeTest(Include "")
AddCMakeTest(FindBase "")
AddCMakeTest(Toolchain "")
AddCMakeTest(GetFilenameComponentRealpath "")
AddCMakeTest(Version "")
AddCMakeTest(Message "")
AddCMakeTest(File "")
AddCMakeTest(ImplicitLinkInfo "")
AddCMakeTest(ModuleNotices "")
AddCMakeTest(GetProperty "")
AddCMakeTest(If "")
AddCMakeTest(String "")
AddCMakeTest(Math "")
AddCMakeTest(CMakeMinimumRequired "")
AddCMakeTest(CompilerIdVendor "")
AddCMakeTest(ProcessorCount "-DKWSYS_TEST_EXE=$<TARGET_FILE:cmsysTestsCxx>")
AddCMakeTest(PushCheckState "")
AddCMakeTest(While "")

AddCMakeTest(FileUpload "")

set(EndStuff_PreArgs
  "-Ddir:STRING=${CMAKE_CURRENT_BINARY_DIR}/EndStuffTest"
  )
AddCMakeTest(EndStuff "${EndStuff_PreArgs}")

AddCMakeTest(GetPrerequisites "-DConfiguration:STRING=$<CONFIGURATION>")

if(GIT_EXECUTABLE)
  set(PolicyCheck_PreArgs
    "-DCMake_BINARY_DIR:PATH=${CMake_BINARY_DIR}"
    "-DCMake_SOURCE_DIR:PATH=${CMake_SOURCE_DIR}"
    "-DGIT_EXECUTABLE:STRING=${GIT_EXECUTABLE}"
    )
  AddCMakeTest(PolicyCheck "${PolicyCheck_PreArgs}")
endif()
