enable_language(C)

add_executable(comments_with_dollars hello.c)
add_custom_command(TARGET comments_with_dollars PRE_LINK
        COMMAND "${CMAKE_COMMAND}" -E echo prelink
        COMMENT "prelink with
\$dollars")
add_custom_command(TARGET comments_with_dollars POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E echo postbuild
        COMMENT "postbuild with
\$dollars")
