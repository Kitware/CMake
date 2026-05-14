# Predicate: returns TRUE for items starting with "b"
function(starts_with_b input result)
  if(input MATCHES "^b")
    set(${result} TRUE PARENT_SCOPE)
  else()
    set(${result} FALSE PARENT_SCOPE)
  endif()
endfunction()

set(mylist alpha bravo charlie bravo_two delta)

# Basic PREDICATE with TOUPPER - only items starting with "b" are uppercased
list(TRANSFORM mylist TOUPPER PREDICATE starts_with_b OUTPUT_VARIABLE output)
if(NOT output STREQUAL "alpha;BRAVO;charlie;BRAVO_TWO;delta")
  message(FATAL_ERROR "TRANSFORM(TOUPPER PREDICATE) is \"${output}\", expected \"alpha;BRAVO;charlie;BRAVO_TWO;delta\"")
endif()

# Verify original list unchanged (OUTPUT_VARIABLE)
if(NOT mylist STREQUAL "alpha;bravo;charlie;bravo_two;delta")
  message(FATAL_ERROR "Original list modified: \"${mylist}\"")
endif()

# PREDICATE in-place
list(TRANSFORM mylist TOUPPER PREDICATE starts_with_b)
if(NOT mylist STREQUAL "alpha;BRAVO;charlie;BRAVO_TWO;delta")
  message(FATAL_ERROR "TRANSFORM(TOUPPER PREDICATE in-place) is \"${mylist}\", expected \"alpha;BRAVO;charlie;BRAVO_TWO;delta\"")
endif()

# PREDICATE combined with APPLY
function(add_prefix in out)
  set(${out} "prefix_${in}" PARENT_SCOPE)
endfunction()

set(mylist alpha bravo charlie delta)
list(TRANSFORM mylist APPLY add_prefix PREDICATE starts_with_b OUTPUT_VARIABLE output)
if(NOT output STREQUAL "alpha;prefix_bravo;charlie;delta")
  message(FATAL_ERROR "TRANSFORM(APPLY PREDICATE) is \"${output}\", expected \"alpha;prefix_bravo;charlie;delta\"")
endif()

# PREDICATE on empty list
set(empty_list "")
list(TRANSFORM empty_list TOUPPER PREDICATE starts_with_b OUTPUT_VARIABLE output)
if(NOT output STREQUAL "")
  message(FATAL_ERROR "TRANSFORM(PREDICATE empty) is \"${output}\", expected \"\"")
endif()

# PREDICATE where nothing matches (all elements unchanged)
set(mylist alpha charlie delta)
list(TRANSFORM mylist TOUPPER PREDICATE starts_with_b OUTPUT_VARIABLE output)
if(NOT output STREQUAL "alpha;charlie;delta")
  message(FATAL_ERROR "TRANSFORM(PREDICATE no-match) is \"${output}\", expected \"alpha;charlie;delta\"")
endif()

# PREDICATE where everything matches
set(mylist bravo bronze bull)
list(TRANSFORM mylist TOUPPER PREDICATE starts_with_b OUTPUT_VARIABLE output)
if(NOT output STREQUAL "BRAVO;BRONZE;BULL")
  message(FATAL_ERROR "TRANSFORM(PREDICATE all-match) is \"${output}\", expected \"BRAVO;BRONZE;BULL\"")
endif()

# Recursive PREDICATE: a PREDICATE function that itself calls list(TRANSFORM APPLY).
# The inner function returns the output variable name it was given.
function(return_pred_var_name in out)
  set(${out} "${out}" PARENT_SCOPE)
endfunction()

# The outer predicate triggers a nested APPLY and verifies that the inner
# output variable name differs from its own result variable.
function(inner_pred_name_is_different input result)
  set(_inner x)
  list(TRANSFORM _inner APPLY return_pred_var_name OUTPUT_VARIABLE _inner_names)
  if("${result}" STREQUAL "${_inner_names}")
    set(${result} FALSE PARENT_SCOPE)
  else()
    set(${result} TRUE PARENT_SCOPE)
  endif()
endfunction()

set(mylist x y z)
list(TRANSFORM mylist TOUPPER PREDICATE inner_pred_name_is_different OUTPUT_VARIABLE output)
if(NOT output STREQUAL "X;Y;Z")
  message(FATAL_ERROR "TRANSFORM(PREDICATE nested smoke) is \"${output}\", expected \"X;Y;Z\"")
endif()
