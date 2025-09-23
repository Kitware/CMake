cmake_minimum_required(VERSION 3.30)
project(PolicyScope C)

#-----------------------------------------------------------------------------
# Helper function to report results.
function(check msg lhs rhs)
  if(NOT "${lhs}" STREQUAL "${rhs}")
    message(FATAL_ERROR "${msg}: expected [${lhs}], got [${rhs}]")
  endif()
endfunction()

#-----------------------------------------------------------------------------
# Put the test modules in the search path.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# Make sure an included file cannot change policies.
include(Bar)
cmake_policy(GET CMP0180 cmp)
check(CMP0180 "" "${cmp}")

# Allow the included file to change policies.
include(Bar NO_POLICY_SCOPE)
cmake_policy(GET CMP0180 cmp)
check(CMP0180 "NEW" "${cmp}")

#-----------------------------------------------------------------------------
# Test function and macro policy recording.

# Create the functions in an isolated scope in which we change policies.
cmake_policy(PUSH)
if(1)
  # Change CMP0179
  cmake_policy(SET CMP0179 OLD)
  function(func1)
    # CMP0179 should be changed when this function is invoked
    cmake_policy(GET CMP0179 cmp)
    check(CMP0179 "OLD" "${cmp}")

    # The undocumented PARENT_SCOPE option sees the caller's setting.
    cmake_policy(GET CMP0179 cmp PARENT_SCOPE)
    check(CMP0179 "NEW" "${cmp}")
  endfunction()

  # Unset CMP0179
  cmake_policy(VERSION 3.30)
  macro(macro1)
    # CMP0179 should be unset when this macro is invoked
    cmake_policy(GET CMP0179 cmp)
    check(CMP0179 "" "${cmp}")

    # The undocumented PARENT_SCOPE option sees the caller's setting.
    cmake_policy(GET CMP0179 cmp PARENT_SCOPE)
    check(CMP0179 "NEW" "${cmp}")

    # Setting the policy should work here and also in the caller.
    cmake_policy(SET CMP0179 OLD)
    cmake_policy(GET CMP0179 cmp)
    check(CMP0179 "OLD" "${cmp}")
  endmacro()
endif()
cmake_policy(POP)

# CMP0179 should still be NEW in this context.
cmake_policy(GET CMP0179 cmp)
check(CMP0179 "NEW" "${cmp}")

# Check the recorded policies
func1()
macro1()

# The macro should have changed CMP0179.
cmake_policy(GET CMP0179 cmp)
check(CMP0179 "OLD" "${cmp}")

#-----------------------------------------------------------------------------
# Test CMAKE_POLICY_DEFAULT_CMP<NNNN> variable.
cmake_policy(PUSH)
  set(CMAKE_POLICY_DEFAULT_CMP0170 OLD) # ignored
  set(CMAKE_POLICY_DEFAULT_CMP0171 OLD) # honored
  set(CMAKE_POLICY_DEFAULT_CMP0172 NEW) # honored
  set(CMAKE_POLICY_DEFAULT_CMP0173 "")  # noop
  cmake_policy(VERSION 3.30)
  cmake_policy(GET CMP0170 cmp)
  check(CMP0170 "NEW" "${cmp}")
  cmake_policy(GET CMP0171 cmp)
  check(CMP0171 "OLD" "${cmp}")
  cmake_policy(GET CMP0172 cmp)
  check(CMP0172 "NEW" "${cmp}")
  cmake_policy(GET CMP0173 cmp)
  check(CMP0173 "" "${cmp}")
cmake_policy(POP)

#-----------------------------------------------------------------------------
# Dummy executable so the project can build and run.
add_executable(PolicyScope main.c)
