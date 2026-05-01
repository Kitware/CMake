# Predicate: returns TRUE for items starting with "FILTER_THIS_"
function(starts_with_filter input result)
  if(input MATCHES "^FILTER_THIS_")
    set(${result} TRUE PARENT_SCOPE)
  else()
    set(${result} FALSE PARENT_SCOPE)
  endif()
endfunction()

set(mylist FILTER_THIS_BIT DO_NOT_FILTER_THIS thisisanitem FILTER_THIS_THING)
message("mylist was: ${mylist}")
list(FILTER mylist EXCLUDE PREDICATE starts_with_filter)
message("mylist is: ${mylist}")

# EXCLUDE on empty list
set(empty_list "")
list(FILTER empty_list EXCLUDE PREDICATE starts_with_filter)
if(NOT empty_list STREQUAL "")
  message(FATAL_ERROR "FILTER(EXCLUDE PREDICATE empty) is \"${empty_list}\", expected \"\"")
endif()

# EXCLUDE where nothing matches (all elements kept)
set(mylist alpha bravo charlie)
list(FILTER mylist EXCLUDE PREDICATE starts_with_filter)
if(NOT mylist STREQUAL "alpha;bravo;charlie")
  message(FATAL_ERROR "FILTER(EXCLUDE PREDICATE no-match) is \"${mylist}\", expected \"alpha;bravo;charlie\"")
endif()

# EXCLUDE where everything matches (all elements removed)
set(mylist FILTER_THIS_A FILTER_THIS_B FILTER_THIS_C)
list(FILTER mylist EXCLUDE PREDICATE starts_with_filter)
if(NOT mylist STREQUAL "")
  message(FATAL_ERROR "FILTER(EXCLUDE PREDICATE all-match) is \"${mylist}\", expected \"\"")
endif()
