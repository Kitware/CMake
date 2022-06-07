# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CMakePrintHelpers
-----------------

Convenience functions for printing properties and variables, useful
e.g. for debugging.

::

  cmake_print_properties(<TARGETS       [<target1> ...] |
                          SOURCES       [<source1> ...] |
                          DIRECTORIES   [<dir1> ...]    |
                          TESTS         [<test1> ...]   |
                          CACHE_ENTRIES [<entry1> ...]  >
                         PROPERTIES [<prop1> ...]         )

This function prints the values of the properties of the given targets,
source files, directories, tests or cache entries.  Exactly one of the
scope keywords must be used.  The scope keyword and its arguments must
come before the ``PROPERTIES`` keyword, in the arguments list.

Example::

  cmake_print_properties(TARGETS foo bar PROPERTIES
                         LOCATION INTERFACE_INCLUDE_DIRECTORIES)

This will print the LOCATION and INTERFACE_INCLUDE_DIRECTORIES properties for
both targets foo and bar.

::

  cmake_print_variables(var1 var2 ..  varN)

This function will print the name of each variable followed by its value.
Example::

  cmake_print_variables(CMAKE_C_COMPILER CMAKE_MAJOR_VERSION DOES_NOT_EXIST)

Gives::

  -- CMAKE_C_COMPILER="/usr/bin/gcc" ; CMAKE_MAJOR_VERSION="2" ; DOES_NOT_EXIST=""
#]=======================================================================]

function(cmake_print_variables)
  set(msg "")
  foreach(var ${ARGN})
    if(msg)
      string(APPEND msg " ; ")
    endif()
    string(APPEND msg "${var}=\"${${var}}\"")
  endforeach()
  message(STATUS "${msg}")
endfunction()


function(cmake_print_properties)
  set(options )
  set(oneValueArgs )
  set(cpp_multiValueArgs PROPERTIES )
  set(cppmode_multiValueArgs TARGETS SOURCES TESTS DIRECTORIES CACHE_ENTRIES )

  string(JOIN " " _mode_names ${cppmode_multiValueArgs})
  set(_missing_mode_message
    "Mode keyword missing in cmake_print_properties() call, there must be exactly one of ${_mode_names}")

  cmake_parse_arguments(
    CPP "${options}" "${oneValueArgs}" "${cpp_multiValueArgs}" ${ARGN})

  if(NOT CPP_PROPERTIES)
    message(FATAL_ERROR
      "Required argument PROPERTIES missing in cmake_print_properties() call")
    return()
  endif()

  if(NOT CPP_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "${_missing_mode_message}")
    return()
  endif()

  cmake_parse_arguments(
    CPPMODE "${options}" "${oneValueArgs}" "${cppmode_multiValueArgs}"
    ${CPP_UNPARSED_ARGUMENTS})

  if(CPPMODE_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR
      "Unknown keywords given to cmake_print_properties(): \"${CPPMODE_UNPARSED_ARGUMENTS}\"")
    return()
  endif()

  set(mode)
  set(items)
  set(keyword)

  if(CPPMODE_TARGETS)
    set(items ${CPPMODE_TARGETS})
    set(mode ${mode} TARGETS)
    set(keyword TARGET)
  endif()

  if(CPPMODE_SOURCES)
    set(items ${CPPMODE_SOURCES})
    set(mode ${mode} SOURCES)
    set(keyword SOURCE)
  endif()

  if(CPPMODE_TESTS)
    set(items ${CPPMODE_TESTS})
    set(mode ${mode} TESTS)
    set(keyword TEST)
  endif()

  if(CPPMODE_DIRECTORIES)
    set(items ${CPPMODE_DIRECTORIES})
    set(mode ${mode} DIRECTORIES)
    set(keyword DIRECTORY)
  endif()

  if(CPPMODE_CACHE_ENTRIES)
    set(items ${CPPMODE_CACHE_ENTRIES})
    set(mode ${mode} CACHE_ENTRIES)
    # This is a workaround for the fact that passing `CACHE` as an argument to
    # set() causes a cache variable to be set.
    set(keyword "")
    string(APPEND keyword CACHE)
  endif()

  if(NOT mode)
    message(FATAL_ERROR "${_missing_mode_message}")
    return()
  endif()

  list(LENGTH mode modeLength)
  if("${modeLength}" GREATER 1)
    message(FATAL_ERROR
      "Multiple mode keywords used in cmake_print_properties() call, there must be exactly one of ${_mode_names}.")
    return()
  endif()

  set(msg "\n")
  foreach(item ${items})

    set(itemExists TRUE)
    if(keyword STREQUAL "TARGET")
      if(NOT TARGET ${item})
        set(itemExists FALSE)
        string(APPEND msg "\n No such TARGET \"${item}\" !\n\n")
      endif()
    endif()

    if (itemExists)
      string(APPEND msg " Properties for ${keyword} ${item}:\n")
      foreach(prop ${CPP_PROPERTIES})

        get_property(propertySet ${keyword} ${item} PROPERTY "${prop}" SET)

        if(propertySet)
          get_property(property ${keyword} ${item} PROPERTY "${prop}")
          string(APPEND msg "   ${item}.${prop} = \"${property}\"\n")
        else()
          string(APPEND msg "   ${item}.${prop} = <NOTFOUND>\n")
        endif()
      endforeach()
    endif()

  endforeach()
  message(STATUS "${msg}")

endfunction()
