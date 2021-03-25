configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sourcefile.txt
    ${CMAKE_CURRENT_BINARY_DIR}/sourcefile-source-permissions.txt
    FILE_PERMISSIONS
        OWNER_READ OWNER_RX
        GROUP_RWX
)
