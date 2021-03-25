set(file_name  ${CMAKE_CURRENT_BINARY_DIR}/escape_quotes.txt)
set(var "\t")
set(ref "${var}")
file(CONFIGURE
    CONTENT "-->@ref@<--"
    OUTPUT ${file_name}
    ESCAPE_QUOTES
)
file(READ ${file_name} file_content)
if(NOT file_content MATCHES "^-->\t<--$")
    message(FATAL_ERROR "ESCAPE_QUOTES doesn't work")
endif()
