file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<LOWER_CASE:$<CONFIG>>/nosourcepermissions.txt"
  INPUT "${CMAKE_CURRENT_SOURCE_DIR}/input.txt"
  NO_SOURCE_PERMISSIONS
  )

add_custom_target(checkNoSourcePermission ALL
  COMMAND ${CMAKE_COMMAND}
    -DgeneratedFile=${CMAKE_CURRENT_BINARY_DIR}/$<LOWER_CASE:$<CONFIG>>/nosourcepermissions.txt
    -P "${CMAKE_CURRENT_SOURCE_DIR}/NoSourcePermissionsVerify.cmake"
  )
