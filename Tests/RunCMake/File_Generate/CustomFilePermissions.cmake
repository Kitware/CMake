file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/customfilepermissions.txt")

file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/$<LOWER_CASE:$<CONFIG>>/customfilepermissions.txt"
  INPUT "${CMAKE_CURRENT_SOURCE_DIR}/input.txt"
  FILE_PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_EXECUTE
    WORLD_EXECUTE
  )

add_custom_target(checkCustomFilePermissions ALL
  COMMAND ${CMAKE_COMMAND}
    -DgeneratedFile=${CMAKE_CURRENT_BINARY_DIR}/$<LOWER_CASE:$<CONFIG>>/customfilepermissions.txt
    -DMSYS=${MSYS}
    -P "${CMAKE_CURRENT_SOURCE_DIR}/CustomFilePermissionsVerify.cmake"
  )
