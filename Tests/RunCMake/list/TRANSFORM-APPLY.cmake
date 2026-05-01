# Define a transform function: add source prefix
function(add_src_prefix in out)
  set(${out} "src/${in}" PARENT_SCOPE)
endfunction()

# Define a transform function: add prefix
function(add_prefix in out)
  set(${out} "prefix_${in}" PARENT_SCOPE)
endfunction()

set(mylist alpha bravo charlie delta)

# Basic APPLY - all elements
list(TRANSFORM mylist APPLY add_src_prefix OUTPUT_VARIABLE output)
if(NOT output STREQUAL "src/alpha;src/bravo;src/charlie;src/delta")
  message(FATAL_ERROR "TRANSFORM(APPLY) is \"${output}\", expected is \"src/alpha;src/bravo;src/charlie;src/delta\"")
endif()

# APPLY with OUTPUT_VARIABLE (verify original unchanged)
if(NOT mylist STREQUAL "alpha;bravo;charlie;delta")
  message(FATAL_ERROR "Original list modified: \"${mylist}\", expected \"alpha;bravo;charlie;delta\"")
endif()

# APPLY in-place
list(TRANSFORM mylist APPLY add_src_prefix)
if(NOT mylist STREQUAL "src/alpha;src/bravo;src/charlie;src/delta")
  message(FATAL_ERROR "TRANSFORM(APPLY) in-place is \"${mylist}\", expected is \"src/alpha;src/bravo;src/charlie;src/delta\"")
endif()

# APPLY with AT selector
set(mylist alpha bravo charlie delta)
list(TRANSFORM mylist APPLY add_src_prefix AT 1 3 OUTPUT_VARIABLE output)
if(NOT output STREQUAL "alpha;src/bravo;charlie;src/delta")
  message(FATAL_ERROR "TRANSFORM(APPLY AT) is \"${output}\", expected is \"alpha;src/bravo;charlie;src/delta\"")
endif()

# APPLY with AT selector and negative index
unset(output)
list(TRANSFORM mylist APPLY add_src_prefix AT 1 -2 OUTPUT_VARIABLE output)
if(NOT output STREQUAL "alpha;src/bravo;src/charlie;delta")
  message(FATAL_ERROR "TRANSFORM(APPLY AT neg) is \"${output}\", expected is \"alpha;src/bravo;src/charlie;delta\"")
endif()

# APPLY with FOR selector
unset(output)
list(TRANSFORM mylist APPLY add_src_prefix FOR 1 2 OUTPUT_VARIABLE output)
if(NOT output STREQUAL "alpha;src/bravo;src/charlie;delta")
  message(FATAL_ERROR "TRANSFORM(APPLY FOR) is \"${output}\", expected is \"alpha;src/bravo;src/charlie;delta\"")
endif()

# APPLY with FOR selector and step
unset(output)
list(TRANSFORM mylist APPLY add_src_prefix FOR 0 3 2 OUTPUT_VARIABLE output)
if(NOT output STREQUAL "src/alpha;bravo;src/charlie;delta")
  message(FATAL_ERROR "TRANSFORM(APPLY FOR step) is \"${output}\", expected is \"src/alpha;bravo;src/charlie;delta\"")
endif()

# APPLY with REGEX selector
unset(output)
list(TRANSFORM mylist APPLY add_src_prefix REGEX "(r|t)a" OUTPUT_VARIABLE output)
if(NOT output STREQUAL "alpha;src/bravo;charlie;src/delta")
  message(FATAL_ERROR "TRANSFORM(APPLY REGEX) is \"${output}\", expected is \"alpha;src/bravo;charlie;src/delta\"")
endif()

# APPLY with a different function
unset(output)
list(TRANSFORM mylist APPLY add_prefix OUTPUT_VARIABLE output)
if(NOT output STREQUAL "prefix_alpha;prefix_bravo;prefix_charlie;prefix_delta")
  message(FATAL_ERROR "TRANSFORM(APPLY add_prefix) is \"${output}\", expected is \"prefix_alpha;prefix_bravo;prefix_charlie;prefix_delta\"")
endif()

# APPLY on empty list
set(empty_list "")
list(TRANSFORM empty_list APPLY add_src_prefix OUTPUT_VARIABLE output)
if(NOT output STREQUAL "")
  message(FATAL_ERROR "TRANSFORM(APPLY empty) is \"${output}\", expected is \"\"")
endif()

# APPLY with function that returns empty string
function(make_empty in out)
  set(${out} "" PARENT_SCOPE)
endfunction()

set(mylist alpha bravo charlie)
list(TRANSFORM mylist APPLY make_empty)
if(NOT mylist STREQUAL ";;")
  message(FATAL_ERROR "TRANSFORM(APPLY make_empty) is \"${mylist}\", expected is \";;\"")
endif()

# Recursive APPLY: an APPLY function that itself calls list(TRANSFORM APPLY).
# The inner function returns the output variable name it was given.
function(return_out_var_name in out)
  set(${out} "${out}" PARENT_SCOPE)
endfunction()

# The outer function triggers a nested APPLY and verifies that the inner
# output variable name differs from its own.
function(inner_name_is_different in out)
  set(_inner x)
  list(TRANSFORM _inner APPLY return_out_var_name OUTPUT_VARIABLE _inner_out)
  # _inner_out now holds the inner output variable name
  if("${out}" STREQUAL "${_inner_out}")
    set(${out} "FALSE" PARENT_SCOPE)
  else()
    set(${out} "TRUE" PARENT_SCOPE)
  endif()
endfunction()

set(mylist a b c)
list(TRANSFORM mylist APPLY inner_name_is_different OUTPUT_VARIABLE output)
if(NOT output STREQUAL "TRUE;TRUE;TRUE")
  message(FATAL_ERROR "TRANSFORM(APPLY nested smoke) is \"${output}\", expected \"TRUE;TRUE;TRUE\"")
endif()
