# A PREDICATE function that reenters list(TRANSFORM) must not corrupt the
# outer transform.  The nested action must match the outer one; a different
# action uses separate state.  Each case checks the nested list too, so
# repairing the outer call by breaking the inner one still fails.

# REPLACE, single element
function(pred_replace value out)
  set(inner "hello")
  list(TRANSFORM inner REPLACE "l" "L")
  if(NOT inner STREQUAL "heLLo")
    message(FATAL_ERROR "nested REPLACE is \"${inner}\", expected \"heLLo\"")
  endif()
  set(${out} TRUE PARENT_SCOPE)
endfunction()

set(replace_single "aXa")
list(TRANSFORM replace_single REPLACE "X" "Z" PREDICATE pred_replace)
if(NOT replace_single STREQUAL "aZa")
  message(FATAL_ERROR "replace_single is \"${replace_single}\", expected \"aZa\"")
endif()

# REPLACE, only the last element reenters
# Nothing follows the reentering element: wrong value, not undefined behavior.
function(pred_replace_last value out)
  if(value STREQUAL "cXc")
    set(inner "hello")
    list(TRANSFORM inner REPLACE "l" "L")
    if(NOT inner STREQUAL "heLLo")
      message(FATAL_ERROR "nested REPLACE is \"${inner}\", expected \"heLLo\"")
    endif()
  endif()
  set(${out} TRUE PARENT_SCOPE)
endfunction()

set(replace_last "aXa" "bXb" "cXc")
list(TRANSFORM replace_last REPLACE "X" "Z" PREDICATE pred_replace_last)
if(NOT replace_last STREQUAL "aZa;bZb;cZc")
  message(FATAL_ERROR "replace_last is \"${replace_last}\", expected \"aZa;bZb;cZc\"")
endif()

# APPEND
# Not redundant with REPLACE: the operand is a plain member, not a
# heap-allocated helper.
function(pred_append value out)
  set(inner "q")
  list(TRANSFORM inner APPEND "_NESTED")
  if(NOT inner STREQUAL "q_NESTED")
    message(FATAL_ERROR "nested APPEND is \"${inner}\", expected \"q_NESTED\"")
  endif()
  set(${out} TRUE PARENT_SCOPE)
endfunction()

set(append_single "a")
list(TRANSFORM append_single APPEND "_OUTER" PREDICATE pred_append)
if(NOT append_single STREQUAL "a_OUTER")
  message(FATAL_ERROR "append_single is \"${append_single}\", expected \"a_OUTER\"")
endif()

# PREPEND
function(pred_prepend value out)
  set(inner "q")
  list(TRANSFORM inner PREPEND "NESTED_")
  if(NOT inner STREQUAL "NESTED_q")
    message(FATAL_ERROR "nested PREPEND is \"${inner}\", expected \"NESTED_q\"")
  endif()
  set(${out} TRUE PARENT_SCOPE)
endfunction()

set(prepend_single "a")
list(TRANSFORM prepend_single PREPEND "OUTER_" PREDICATE pred_prepend)
if(NOT prepend_single STREQUAL "OUTER_a")
  message(FATAL_ERROR "prepend_single is \"${prepend_single}\", expected \"OUTER_a\"")
endif()

# TOUPPER, every element reenters
# The only case transforming an element after a reentering one, covering a
# selector that outlives the nested call.
function(pred_toupper value out)
  set(inner a b)
  list(TRANSFORM inner TOUPPER)
  if(NOT inner STREQUAL "A;B")
    message(FATAL_ERROR "nested TOUPPER is \"${inner}\", expected \"A;B\"")
  endif()
  set(${out} TRUE PARENT_SCOPE)
endfunction()

set(toupper_all x y z)
list(TRANSFORM toupper_all TOUPPER PREDICATE pred_toupper)
if(NOT toupper_all STREQUAL "X;Y;Z")
  message(FATAL_ERROR "toupper_all is \"${toupper_all}\", expected \"X;Y;Z\"")
endif()
