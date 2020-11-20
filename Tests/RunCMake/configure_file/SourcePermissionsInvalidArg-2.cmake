configure_file(sourcefile.txt
    ${CMAKE_CURRENT_BINARY_DIR}/sourcefile.txt
    NO_SOURCE_PERMISSIONS
    FILE_PERMISSIONS OWNER_READ
)
