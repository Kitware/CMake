set(file_name  ${CMAKE_CURRENT_BINARY_DIR}/NewLineStyle.txt)

function(test_eol style in out)
    file(CONFIGURE
        OUTPUT ${file_name}
        CONTENT "@in@"
        NEWLINE_STYLE ${style}
    )
    file(READ ${file_name} new HEX)
    if(NOT "${new}" STREQUAL "${out}")
        message(FATAL_ERROR "No ${style} line endings")
    endif()
endfunction()

test_eol(DOS   "a" "610d0a")
test_eol(WIN32 "b" "620d0a")
test_eol(CRLF  "c" "630d0a")

test_eol(UNIX  "d" "640a")
test_eol(LF    "e" "650a")
