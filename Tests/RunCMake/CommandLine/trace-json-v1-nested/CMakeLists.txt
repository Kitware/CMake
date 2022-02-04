function(f)
    message(STATUS "nested global_frame")
endfunction()

function(g)
    f()
endfunction()

g()
