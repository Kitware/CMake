set(ExternalData_STATE_ROOT "${CMAKE_CURRENT_BINARY_DIR}/State")
ExternalData_Add_Test(DataStateRoot
  NAME DataStateRootCheck
  COMMAND ${CMAKE_COMMAND}
    -D Data=DATA{Data.dat}
    -D AdjacentHash=${ExternalData_BINARY_ROOT}/DataStateRoot/Data.dat-hash-stamp
    -D StateHash=${ExternalData_STATE_ROOT}/DataStateRoot/Data.dat-hash-stamp
    -P ${CMAKE_CURRENT_SOURCE_DIR}/DataStateRootCheck.cmake
  )
ExternalData_Add_Target(DataStateRoot)
