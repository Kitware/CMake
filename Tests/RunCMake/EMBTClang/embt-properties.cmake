enable_language(C)

function(expect_prop tgt prop want)
  get_property(got TARGET ${tgt} PROPERTY ${prop})
  if(NOT "${got}" STREQUAL "${want}")
    message(SEND_ERROR "${tgt} ${prop}: expected '${want}', got '${got}'")
  endif()
endfunction()

# A target created before the variable is set must not inherit it.
add_library(before empty.c)
expect_prop(before EMBT_RUNTIME_LIBRARY "")

# The CMAKE_EMBT_RUNTIME_LIBRARY variable initializes the property at creation.
set(CMAKE_EMBT_RUNTIME_LIBRARY "Dynamic")

add_library(after empty.c)
expect_prop(after EMBT_RUNTIME_LIBRARY "Dynamic")
