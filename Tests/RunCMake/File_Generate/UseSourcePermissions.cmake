file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<LOWER_CASE:$<CONFIG>>/usesourcepermissions.txt"
  INPUT "${CMAKE_CURRENT_SOURCE_DIR}/input.txt"
  USE_SOURCE_PERMISSIONS
  )

add_custom_target(checkUseSourcePermissions ALL
  COMMAND ${CMAKE_COMMAND}
    -DsourceFile=${CMAKE_CURRENT_SOURCE_DIR}/input.txt
    -DgeneratedFile=${CMAKE_CURRENT_BINARY_DIR}/$<LOWER_CASE:$<CONFIG>>/usesourcepermissions.txt
    -P "${CMAKE_CURRENT_SOURCE_DIR}/UseSourcePermissionsVerify.cmake"
  )
