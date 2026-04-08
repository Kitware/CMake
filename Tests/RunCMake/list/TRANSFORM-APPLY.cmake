# Define a transform function: add source prefix
function(add_src_prefix in out)
  set(${out} "src/${in}" PARENT_SCOPE)
endfunction()

# Define a transform function: add prefix
function(add_prefix in out)
  set(${out} "prefix_${in}" PARENT_SCOPE)
endfunction()

# Define a transform macro: wrap in angle brackets
macro(wrap_angles in out)
  set(${out} "<${in}>")
endmacro()

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

# APPLY with a macro
set(mylist alpha bravo charlie)
list(TRANSFORM mylist APPLY wrap_angles OUTPUT_VARIABLE output)
if(NOT output STREQUAL "<alpha>;<bravo>;<charlie>")
  message(FATAL_ERROR "TRANSFORM(APPLY macro) is \"${output}\", expected is \"<alpha>;<bravo>;<charlie>\"")
endif()

# APPLY with macro and selector
list(TRANSFORM mylist APPLY wrap_angles AT 0 2 OUTPUT_VARIABLE output)
if(NOT output STREQUAL "<alpha>;bravo;<charlie>")
  message(FATAL_ERROR "TRANSFORM(APPLY macro AT) is \"${output}\", expected is \"<alpha>;bravo;<charlie>\"")
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
