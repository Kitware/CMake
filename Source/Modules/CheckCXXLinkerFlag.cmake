# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

include_guard(GLOBAL)
include(CheckCXXSourceCompiles)
include(CMakeCheckCompilerFlagCommonPatterns)

function(check_cxx_linker_flag _flag _var)
  if(CMAKE_VERSION VERSION_LESS "3.14")
    set(CMAKE_REQUIRED_LIBRARIES "${_flag}")
  else()
    set(CMAKE_REQUIRED_LINK_OPTIONS "${_flag}")
  endif()

  # Normalize locale during test compilation.
  set(_locale_vars LC_ALL LC_MESSAGES LANG)
  foreach(v IN LISTS _locale_vars)
    set(_locale_vars_saved_${v} "$ENV{${v}}")
    set(ENV{${v}} C)
  endforeach()
  check_compiler_flag_common_patterns(_common_patterns)
  check_cxx_source_compiles("int main() { return 0; }" ${_var}
    ${_common_patterns}
    )
  foreach(v IN LISTS _locale_vars)
    set(ENV{${v}} ${_locale_vars_saved_${v}})
  endforeach()
  set(${_var} "${${_var}}" PARENT_SCOPE)
endfunction()
