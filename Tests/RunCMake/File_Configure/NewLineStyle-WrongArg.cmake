set(file_name  ${CMAKE_CURRENT_BINARY_DIR}/NewLineStyle.txt)
file(CONFIGURE
    OUTPUT ${file_name}
    CONTENT "Data\n"
    NEWLINE_STYLE FOO
)
