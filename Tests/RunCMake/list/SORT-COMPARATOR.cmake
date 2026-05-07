# Comparator: returns TRUE if a < b (string comparison)
function(string_less a b result)
  if("${a}" STRLESS "${b}")
    set(${result} TRUE PARENT_SCOPE)
  else()
    set(${result} FALSE PARENT_SCOPE)
  endif()
endfunction()

# Comparator as macro
macro(string_less_macro a b result)
  if(${a} STRLESS ${b})
    set(${result} TRUE)
  else()
    set(${result} FALSE)
  endif()
endmacro()

# Comparator: string length comparison
function(shorter_first a b result)
  string(LENGTH "${a}" len_a)
  string(LENGTH "${b}" len_b)
  if(len_a LESS len_b)
    set(${result} TRUE PARENT_SCOPE)
  else()
    set(${result} FALSE PARENT_SCOPE)
  endif()
endfunction()

## Basic COMPARATOR with function
set(mylist c a b)
list(SORT mylist COMPARATOR string_less)
if(NOT mylist STREQUAL "a;b;c")
  message(FATAL_ERROR "SORT(COMPARATOR function) is \"${mylist}\", expected \"a;b;c\"")
endif()

## COMPARATOR with macro
set(mylist c a b)
list(SORT mylist COMPARATOR string_less_macro)
if(NOT mylist STREQUAL "a;b;c")
  message(FATAL_ERROR "SORT(COMPARATOR macro) is \"${mylist}\", expected \"a;b;c\"")
endif()

## COMPARATOR with ORDER DESCENDING
set(mylist c a b)
list(SORT mylist COMPARATOR string_less ORDER DESCENDING)
if(NOT mylist STREQUAL "c;b;a")
  message(FATAL_ERROR "SORT(COMPARATOR ORDER DESCENDING) is \"${mylist}\", expected \"c;b;a\"")
endif()

## COMPARATOR with CASE INSENSITIVE
# With CASE INSENSITIVE, values are lowercased before being passed to the comparator
set(mylist "C" "a" "B")
list(SORT mylist COMPARATOR string_less CASE INSENSITIVE)
if(NOT mylist STREQUAL "a;B;C")
  message(FATAL_ERROR "SORT(COMPARATOR CASE INSENSITIVE) is \"${mylist}\", expected \"a;B;C\"")
endif()

## COMPARATOR with CASE INSENSITIVE and ORDER DESCENDING
set(mylist "C" "a" "B")
list(SORT mylist COMPARATOR string_less CASE INSENSITIVE ORDER DESCENDING)
if(NOT mylist STREQUAL "C;B;a")
  message(FATAL_ERROR "SORT(COMPARATOR CASE INSENSITIVE ORDER DESCENDING) is \"${mylist}\", expected \"C;B;a\"")
endif()

## Custom comparator (sort by string length)
set(mylist "bbb" "a" "cc")
list(SORT mylist COMPARATOR shorter_first)
if(NOT mylist STREQUAL "a;cc;bbb")
  message(FATAL_ERROR "SORT(COMPARATOR shorter_first) is \"${mylist}\", expected \"a;cc;bbb\"")
endif()

## Custom comparator with ORDER DESCENDING (sort by length descending)
set(mylist "bbb" "a" "cc")
list(SORT mylist COMPARATOR shorter_first ORDER DESCENDING)
if(NOT mylist STREQUAL "bbb;cc;a")
  message(FATAL_ERROR "SORT(COMPARATOR shorter_first ORDER DESCENDING) is \"${mylist}\", expected \"bbb;cc;a\"")
endif()

## Empty list (no-op)
set(empty_list "")
list(SORT empty_list COMPARATOR string_less)
if(NOT empty_list STREQUAL "")
  message(FATAL_ERROR "SORT(COMPARATOR empty) is \"${empty_list}\", expected \"\"")
endif()

## Single element list (no-op)
set(mylist "only")
list(SORT mylist COMPARATOR string_less)
if(NOT mylist STREQUAL "only")
  message(FATAL_ERROR "SORT(COMPARATOR single) is \"${mylist}\", expected \"only\"")
endif()

## Already sorted list
set(mylist a b c)
list(SORT mylist COMPARATOR string_less)
if(NOT mylist STREQUAL "a;b;c")
  message(FATAL_ERROR "SORT(COMPARATOR already-sorted) is \"${mylist}\", expected \"a;b;c\"")
endif()

## Recursive COMPARATOR - verify inner and outer output variables are distinct
# Inner comparator: checks that neither element equals its own output variable
# name (which would mean the outer and inner names collided), then performs a
# valid string comparison to satisfy strict weak ordering.
function(check_outer_name a b out)
  if("${a}" STREQUAL "${out}")
    message(FATAL_ERROR "Recursive COMPARATOR: inner and outer output variable names are the same: ${out}")
  endif()
  if("${b}" STREQUAL "${out}")
    message(FATAL_ERROR "Recursive COMPARATOR: inner and outer output variable names are the same: ${out}")
  endif()
  if("${a}" STRLESS "${b}")
    set(${out} TRUE PARENT_SCOPE)
  else()
    set(${out} FALSE PARENT_SCOPE)
  endif()
endfunction()

# Outer comparator: passes its own output variable name into a nested sort
# as a list element, so the inner comparator can compare the two names.
function(nested_comparator a b out)
  set(_inner "${out}" dummy)
  list(SORT _inner COMPARATOR check_outer_name)
  if("${a}" STRLESS "${b}")
    set(${out} TRUE PARENT_SCOPE)
  else()
    set(${out} FALSE PARENT_SCOPE)
  endif()
endfunction()

set(mylist c a b)
list(SORT mylist COMPARATOR nested_comparator)
if(NOT mylist STREQUAL "a;b;c")
  message(FATAL_ERROR "SORT(COMPARATOR nested) is \"${mylist}\", expected \"a;b;c\"")
endif()
