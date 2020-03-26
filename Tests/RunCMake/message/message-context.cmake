function(bar)
    list(APPEND CMAKE_MESSAGE_CONTEXT "bar")
    list(APPEND CMAKE_MESSAGE_INDENT "<-- indent -->")
    message(VERBOSE "bar VERBOSE message")
endfunction()

function(baz)
    list(APPEND CMAKE_MESSAGE_CONTEXT "baz")
    message(DEBUG "This is the multi-line\nbaz DEBUG message")
endfunction()

function(foo)
    list(APPEND CMAKE_MESSAGE_CONTEXT "foo")
    bar()
    message(TRACE "foo TRACE message")
    baz()
endfunction()

message(STATUS "Begin context output test")
list(APPEND CMAKE_MESSAGE_CONTEXT "top")

message(STATUS "Top: before")
foo()
message(STATUS "Top: after")

list(POP_BACK CMAKE_MESSAGE_CONTEXT)
message(STATUS "End of context output test")
