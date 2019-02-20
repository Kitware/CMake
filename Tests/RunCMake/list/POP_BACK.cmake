cmake_policy(SET CMP0054 NEW)

function(assert_expected_list_len list_var expected_size)
    list(LENGTH ${list_var} _size)
    if(NOT _size EQUAL ${expected_size})
        message(FATAL_ERROR "list size expected to be `${expected_size}`, got `${_size}` instead")
    endif()
endfunction()

# Pop from undefined list
list(POP_BACK test)
if(DEFINED test)
    message(FATAL_ERROR "`test` expected to be undefined")
endif()

# Pop from empty list
set(test)
list(POP_BACK test)
if(DEFINED test)
    message(FATAL_ERROR "`test` expected to be undefined")
endif()

# Default pop from 1-item list
list(APPEND test one)
list(POP_BACK test)
assert_expected_list_len(test 0)

# Pop from 1-item list to var
list(APPEND test one)
list(POP_BACK test one)
assert_expected_list_len(test 0)
if(NOT DEFINED one)
    message(FATAL_ERROR "`one` expected to be defined")
endif()
if(NOT one STREQUAL "one")
    message(FATAL_ERROR "`one` has unexpected value `${one}`")
endif()

unset(one)
unset(two)

# Pop from 1-item list to vars
list(APPEND test one)
list(POP_BACK test one two)
assert_expected_list_len(test 0)
if(NOT DEFINED one)
    message(FATAL_ERROR "`one` expected to be defined")
endif()
if(NOT one STREQUAL "one")
    message(FATAL_ERROR "`one` has unexpected value `${one}`")
endif()
if(DEFINED two)
    message(FATAL_ERROR "`two` expected to be undefined")
endif()

unset(one)
unset(two)

# Default pop from 2-item list
list(APPEND test one two)
list(POP_BACK test)
assert_expected_list_len(test 1)
if(NOT test STREQUAL "one")
    message(FATAL_ERROR "`test` has unexpected value `${test}`")
endif()

# Pop from 2-item list
list(APPEND test two)
list(POP_BACK test two)
assert_expected_list_len(test 1)
if(NOT DEFINED two)
    message(FATAL_ERROR "`two` expected to be defined")
endif()
if(NOT two STREQUAL "two")
    message(FATAL_ERROR "`two` has unexpected value `${two}`")
endif()
if(NOT test STREQUAL "one")
    message(FATAL_ERROR "`test` has unexpected value `${test}`")
endif()
