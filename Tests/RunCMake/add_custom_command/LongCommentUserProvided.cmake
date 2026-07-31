set(manualComment "Message X times:")
foreach(i RANGE 1 10850)
    set(manualComment "${manualComment} hello")
endforeach()

add_custom_command(
    OUTPUT out.txt
    COMMAND "${CMAKE_COMMAND}" -E touch out.txt
    COMMENT "${manualComment}"
)

add_custom_target(LongCommentUserProvided ALL DEPENDS out.txt)
