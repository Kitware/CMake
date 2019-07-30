list(PREPEND test)
if(test)
    message(FATAL_ERROR "failed")
endif()

list(PREPEND test satu)
if(NOT test STREQUAL "satu")
    message(FATAL_ERROR "failed")
endif()

list(PREPEND test dua)
if(NOT test STREQUAL "dua;satu")
    message(FATAL_ERROR "failed")
endif()

list(PREPEND test tiga)
if(NOT test STREQUAL "tiga;dua;satu")
    message(FATAL_ERROR "failed")
endif()

# Scope test
function(foo)
    list(PREPEND test empat)
    if(NOT test STREQUAL "empat;tiga;dua;satu")
        message(FATAL_ERROR "failed")
    endif()
endfunction()

foo()

if(NOT test STREQUAL "tiga;dua;satu")
    message(FATAL_ERROR "failed")
endif()
