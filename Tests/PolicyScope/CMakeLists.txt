cmake_minimum_required(VERSION 3.0)
project(PolicyScope C)
# No cmake_minimum_required(VERSION), it's in FindFoo.

#-----------------------------------------------------------------------------
# Helper function to report results.
function(check msg lhs rhs)
  if(NOT "${lhs}" STREQUAL "${rhs}")
    message(FATAL_ERROR "${msg}: expected [${lhs}], got [${rhs}]")
  endif()
endfunction()

#-----------------------------------------------------------------------------
# Test using a development framework that sets policies for us.

# Put the test modules in the search path.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# The included file should set policies for us.
find_package(Foo NO_POLICY_SCOPE)

# Check policies set by the package.
cmake_policy(GET CMP0056 cmp)
check(CMP0056 "OLD" "${cmp}")
cmake_policy(GET CMP0055 cmp)
check(CMP0055 "NEW" "${cmp}")

# Make sure an included file cannot change policies.
include(Bar)
cmake_policy(GET CMP0056 cmp)
check(CMP0056 "OLD" "${cmp}")

# Allow the included file to change policies.
include(Bar NO_POLICY_SCOPE)
cmake_policy(GET CMP0056 cmp)
check(CMP0056 "NEW" "${cmp}")

#-----------------------------------------------------------------------------
# Test function and macro policy recording.

# Create the functions in an isolated scope in which we change policies.
cmake_policy(PUSH)
if(1)
  cmake_policy(SET CMP0055 OLD)
  function(func1)
    # CMP0055 should be changed when this function is invoked
    cmake_policy(GET CMP0055 cmp)
    check(CMP0055 "OLD" "${cmp}")
  endfunction()

  # Unset CMP0055
  cmake_policy(VERSION 3.1)
  macro(macro1)
    # CMP0055 should be unset when this macro is invoked
    cmake_policy(GET CMP0055 cmp)
    check(CMP0055 "" "${cmp}")

    # Setting the policy should work here and also in the caller.
    cmake_policy(SET CMP0055 OLD)
    cmake_policy(GET CMP0055 cmp)
    check(CMP0055 "OLD" "${cmp}")
  endmacro()
endif()
cmake_policy(POP)

# CMP0055 should still be NEW in this context.
cmake_policy(GET CMP0055 cmp)
check(CMP0055 "NEW" "${cmp}")

# Check the recorded policies
func1()
macro1()

# The macro should have changed CMP0055.
cmake_policy(GET CMP0055 cmp)
check(CMP0055 "OLD" "${cmp}")

#-----------------------------------------------------------------------------
# Test CMAKE_POLICY_DEFAULT_CMP<NNNN> variable.
cmake_policy(PUSH)
  set(CMAKE_POLICY_DEFAULT_CMP0054 OLD) # ignored
  set(CMAKE_POLICY_DEFAULT_CMP0055 OLD) # honored
  set(CMAKE_POLICY_DEFAULT_CMP0056 NEW) # honored
  set(CMAKE_POLICY_DEFAULT_CMP0057 "")  # noop
  cmake_policy(VERSION 3.1)
  cmake_policy(GET CMP0054 cmp)
  check(CMP0054 "NEW" "${cmp}")
  cmake_policy(GET CMP0055 cmp)
  check(CMP0055 "OLD" "${cmp}")
  cmake_policy(GET CMP0056 cmp)
  check(CMP0056 "NEW" "${cmp}")
  cmake_policy(GET CMP0057 cmp)
  check(CMP0057 "" "${cmp}")
cmake_policy(POP)

#-----------------------------------------------------------------------------
# Dummy executable so the project can build and run.
add_executable(PolicyScope main.c)
