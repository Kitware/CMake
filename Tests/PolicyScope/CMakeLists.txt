cmake_minimum_required(VERSION 2.6.3)
project(PolicyScope C)

#-----------------------------------------------------------------------------
# Helper function to report results.
function(check msg lhs rhs)
  if(NOT "${lhs}" STREQUAL "${rhs}")
    message(FATAL_ERROR "${msg}: expected [${lhs}], got [${rhs}]")
  endif()
endfunction(check)

#-----------------------------------------------------------------------------
# Test function and macro policy recording.

# Create the functions in an isolated scope in which we change policies.
cmake_policy(PUSH)
if(1)
  # Change CMP0002
  cmake_policy(SET CMP0002 OLD)
  function(func1)
    # CMP0002 should be changed when this function is invoked
    cmake_policy(GET CMP0002 cmp)
    check(CMP0002 "OLD" "${cmp}")
  endfunction(func1)

  # Unset CMP0002
  cmake_policy(VERSION 2.4)
  macro(macro1)
    # CMP0002 should be unset when this macro is invoked
    cmake_policy(GET CMP0002 cmp)
    check(CMP0002 "" "${cmp}")

    # Setting the policy should work here and also in the caller.
    cmake_policy(SET CMP0002 OLD)
    cmake_policy(GET CMP0002 cmp)
    check(CMP0002 "OLD" "${cmp}")
  endmacro(macro1)
endif(1)
cmake_policy(POP)

# CMP0002 should still be NEW in this context.
cmake_policy(GET CMP0002 cmp)
check(CMP0002 "NEW" "${cmp}")

# Check the recorded policies
func1()
macro1()

# The macro should have changed CMP0002.
cmake_policy(GET CMP0002 cmp)
check(CMP0002 "OLD" "${cmp}")

#-----------------------------------------------------------------------------
# Dummy executable so the project can build and run.
add_executable(PolicyScope main.c)
