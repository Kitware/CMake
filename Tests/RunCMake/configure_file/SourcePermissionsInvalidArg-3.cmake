configure_file(sourcefile.txt
    ${CMAKE_CURRENT_BINARY_DIR}/sourcefile.txt
    USE_SOURCE_PERMISSIONS
    FILE_PERMISSIONS OWNER_READ
)
