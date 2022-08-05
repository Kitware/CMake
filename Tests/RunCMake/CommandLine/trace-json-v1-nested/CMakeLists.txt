function(function_that_uses_else)
    if(FALSE)
    else()
    endif()
endfunction()
function(f)
    message(STATUS "nested global_frame")
endfunction()

function(g)
    f()
endfunction()

g()
