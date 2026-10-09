if (RunCMake_GENERATOR STREQUAL "Xcode" OR
    NOT CMAKE_C_COMPILER_ID MATCHES "^(GNU|LCC|Clang|AppleClang|IntelLLVM|MSVC)$")
  return()
endif()

# MSVC defines the system header pragma only for toolset 14.13 from VS 2017 15.6.
if (CMAKE_C_COMPILER_ID STREQUAL "MSVC" AND MSVC_VERSION LESS 1913)
  return()
endif()

function(check_system_header_pragma target expected)
  set(pch_header "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/${target}.dir/cmake_pch.h")
  if (RunCMake_GENERATOR_IS_MULTI_CONFIG)
    set(pch_header "${RunCMake_TEST_BINARY_DIR}/CMakeFiles/${target}.dir/Debug/cmake_pch.h")
  endif()

  if (NOT EXISTS "${pch_header}")
    set(RunCMake_TEST_FAILED
      "Generated pch file for '${target}'\n  ${pch_header}\ndoes not exist"
      PARENT_SCOPE)
    return()
  endif()

  file(STRINGS "${pch_header}" pch_header_strings REGEX "system_header")
  if (expected AND NOT pch_header_strings)
    set(RunCMake_TEST_FAILED
      "Generated pch file for '${target}' is missing the system header pragma"
      PARENT_SCOPE)
  elseif (NOT expected AND pch_header_strings)
    set(RunCMake_TEST_FAILED
      "Generated pch file for '${target}' has an unexpected system header pragma"
      PARENT_SCOPE)
  endif()
endfunction()

check_system_header_pragma(empty ON)
if (RunCMake_TEST_FAILED)
  return()
endif()
check_system_header_pragma(foo OFF)
