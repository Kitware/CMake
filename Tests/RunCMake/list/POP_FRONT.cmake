cmake_policy(SET CMP0054 NEW)

function(assert_expected_list_len list_var expected_size)
    list(LENGTH ${list_var} _size)
    if(NOT _size EQUAL ${expected_size})
        message(FATAL_ERROR "list size expected to be `${expected_size}`, got `${_size}` instead")
    endif()
endfunction()

# Pop from undefined list
list(POP_FRONT test)
if(DEFINED test)
    message(FATAL_ERROR "`test` expected to be undefined")
endif()

# Pop from empty list
set(test)
list(POP_FRONT test)
if(DEFINED test)
    message(FATAL_ERROR "`test` expected to be undefined")
endif()

# Default pop from 1-item list
list(APPEND test one)
list(POP_FRONT test)
assert_expected_list_len(test 0)

# Pop from 1-item list to var
list(APPEND test one)
list(POP_FRONT test one)
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
list(POP_FRONT test one two)
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
list(POP_FRONT test)
assert_expected_list_len(test 1)
if(NOT test STREQUAL "two")
    message(FATAL_ERROR "`test` has unexpected value `${test}`")
endif()

# Pop from 2-item list
list(PREPEND test one)
list(POP_FRONT test one)
assert_expected_list_len(test 1)
if(NOT DEFINED one)
    message(FATAL_ERROR "`one` expected to be defined")
endif()
if(NOT one STREQUAL "one")
    message(FATAL_ERROR "`one` has unexpected value `${one}`")
endif()
if(NOT test STREQUAL "two")
    message(FATAL_ERROR "`test` has unexpected value `${test}`")
endif()

# BUG 19436
set(myList a b c)
list(POP_FRONT myList first second)
if(NOT first STREQUAL "a")
    message(FATAL_ERROR "BUG#19436: `first` has unexpected value `${first}`")
endif()
if(NOT second STREQUAL "b")
    message(FATAL_ERROR "BUG#19436: `second` has unexpected value `${second}`")
endif()
if(NOT myList STREQUAL "c")
    message(FATAL_ERROR "BUG#19436: `myList` has unexpected value `${myList}`")
endif()
