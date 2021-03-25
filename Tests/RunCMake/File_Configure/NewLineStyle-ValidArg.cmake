set(file_name  ${CMAKE_CURRENT_BINARY_DIR}/NewLineStyle.txt)

function(test_eol style in out)
    if (style)
      set(newline_stle NEWLINE_STYLE ${style})
    endif()
    file(CONFIGURE
        OUTPUT ${file_name}
        CONTENT "@in@"
        ${newline_stle}
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

if (WIN32)
    test_eol("" "a\nb" "610d0a62")
elseif(UNIX)
    test_eol("" "a\nb" "610a62")
endif()
