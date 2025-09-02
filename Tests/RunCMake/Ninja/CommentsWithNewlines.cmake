enable_language(C)

add_executable(comments_with_newlines hello.c)
add_custom_command(TARGET comments_with_newlines PRE_LINK
        COMMAND "${CMAKE_COMMAND}" -E echo prelink
        COMMENT "prelink with
newline")
add_custom_command(TARGET comments_with_newlines POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E echo postbuild
        COMMENT "postbuild with
newline")
