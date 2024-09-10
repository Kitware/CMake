# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckPrototypeDefinition
------------------------

Check if the prototype we expect is correct.

.. command:: check_prototype_definition

  .. code-block:: cmake

    check_prototype_definition(FUNCTION PROTOTYPE RETURN HEADER VARIABLE)

  ::

    FUNCTION - The name of the function (used to check if prototype exists)
    PROTOTYPE- The prototype to check.
    RETURN - The return value of the function.
    HEADER - The header files required.
    VARIABLE - The variable to store the result.
               Will be created as an internal cache variable.

  Example:

  .. code-block:: cmake

    check_prototype_definition(getpwent_r
     "struct passwd *getpwent_r(struct passwd *src, char *buf, int buflen)"
     "NULL"
     "unistd.h;pwd.h"
     SOLARIS_GETPWENT_R)

The following variables may be set before calling this function to modify
the way the check is run:

.. include:: /module/CMAKE_REQUIRED_FLAGS.txt

.. include:: /module/CMAKE_REQUIRED_DEFINITIONS.txt

.. include:: /module/CMAKE_REQUIRED_INCLUDES.txt

.. include:: /module/CMAKE_REQUIRED_LINK_OPTIONS.txt

.. include:: /module/CMAKE_REQUIRED_LIBRARIES.txt

.. include:: /module/CMAKE_REQUIRED_LINK_DIRECTORIES.txt

.. include:: /module/CMAKE_REQUIRED_QUIET.txt

#]=======================================================================]

#

get_filename_component(__check_proto_def_dir "${CMAKE_CURRENT_LIST_FILE}" PATH)

include_guard(GLOBAL)

function(check_prototype_definition _FUNCTION _PROTOTYPE _RETURN _HEADER _VARIABLE)

  if (NOT DEFINED ${_VARIABLE})
    if(NOT CMAKE_REQUIRED_QUIET)
      message(CHECK_START "Checking prototype ${_FUNCTION} for ${_VARIABLE}")
    endif()
    set(CHECK_PROTOTYPE_DEFINITION_CONTENT "/* */\n")

    set(CHECK_PROTOTYPE_DEFINITION_FLAGS ${CMAKE_REQUIRED_FLAGS})
    if (CMAKE_REQUIRED_LINK_OPTIONS)
      set(CHECK_PROTOTYPE_DEFINITION_LINK_OPTIONS
        LINK_OPTIONS ${CMAKE_REQUIRED_LINK_OPTIONS})
    else()
      set(CHECK_PROTOTYPE_DEFINITION_LINK_OPTIONS)
    endif()
    if (CMAKE_REQUIRED_LIBRARIES)
      set(CHECK_PROTOTYPE_DEFINITION_LIBS
        LINK_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
    else()
      set(CHECK_PROTOTYPE_DEFINITION_LIBS)
    endif()
    if (CMAKE_REQUIRED_INCLUDES)
      set(CMAKE_SYMBOL_EXISTS_INCLUDES
        "-DINCLUDE_DIRECTORIES:STRING=${CMAKE_REQUIRED_INCLUDES}")
    else()
      set(CMAKE_SYMBOL_EXISTS_INCLUDES)
    endif()

    if(CMAKE_REQUIRED_LINK_DIRECTORIES)
      set(_CPD_LINK_DIRECTORIES
        "-DLINK_DIRECTORIES:STRING=${CMAKE_REQUIRED_LINK_DIRECTORIES}")
    else()
      set(_CPD_LINK_DIRECTORIES)
    endif()

    foreach(_FILE ${_HEADER})
      string(APPEND CHECK_PROTOTYPE_DEFINITION_HEADER
        "#include <${_FILE}>\n")
    endforeach()

    set(CHECK_PROTOTYPE_DEFINITION_SYMBOL ${_FUNCTION})
    set(CHECK_PROTOTYPE_DEFINITION_PROTO ${_PROTOTYPE})
    set(CHECK_PROTOTYPE_DEFINITION_RETURN ${_RETURN})

    file(READ ${__check_proto_def_dir}/CheckPrototypeDefinition.c.in _SOURCE)
    string(CONFIGURE "${_SOURCE}" _SOURCE @ONLY)

    try_compile(${_VARIABLE}
      SOURCE_FROM_VAR CheckPrototypeDefinition.c _SOURCE
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      ${CHECK_PROTOTYPE_DEFINITION_LINK_OPTIONS}
      ${CHECK_PROTOTYPE_DEFINITION_LIBS}
      CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${CHECK_PROTOTYPE_DEFINITION_FLAGS}
      "${CMAKE_SYMBOL_EXISTS_INCLUDES}"
      "${_CPD_LINK_DIRECTORIES}"
      )
    unset(_CPD_LINK_DIRECTORIES)

    if (${_VARIABLE})
      set(${_VARIABLE} 1 CACHE INTERNAL "Have correct prototype for ${_FUNCTION}")
      if(NOT CMAKE_REQUIRED_QUIET)
        message(CHECK_PASS "True")
      endif()
    else ()
      if(NOT CMAKE_REQUIRED_QUIET)
        message(CHECK_FAIL "False")
      endif()
      set(${_VARIABLE} 0 CACHE INTERNAL "Have correct prototype for ${_FUNCTION}")
    endif ()
  endif()

endfunction()
