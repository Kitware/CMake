# Test adding the same data file in multiple tests
ExternalData_Add_Test(Data5.A
  NAME Data5Check.A
  COMMAND ${CMAKE_COMMAND}
    -D Data5=DATA{Data5.dat}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/Data5Check.cmake
  )
ExternalData_Add_Target(Data5.A)

if(CMAKE_XCODE_BUILD_SYSTEM VERSION_GREATER_EQUAL 12)
  # Xcode's "new build system" does not support multiple targets
  # producing the same custom command output.
  return()
endif()

ExternalData_Add_Test(Data5.B
  NAME Data5Check.B
  COMMAND ${CMAKE_COMMAND}
    -D Data5=DATA{Data5.dat}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/Data5Check.cmake
  )
ExternalData_Add_Target(Data5.B)
ExternalData_Add_Test(Data5.C
  NAME Data5Check.C
  COMMAND ${CMAKE_COMMAND}
    -D Data5=DATA{Data5.dat}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/Data5Check.cmake
  )
ExternalData_Add_Target(Data5.C)
