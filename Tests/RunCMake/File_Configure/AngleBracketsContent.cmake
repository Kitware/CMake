file(CONFIGURE
    OUTPUT "file.txt"
    CONTENT "foo-$<CONFIG>"
)
file(READ ${CMAKE_CURRENT_BINARY_DIR}/file.txt out)
message("${out}")
