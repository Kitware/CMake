# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CMakeDependentOption
--------------------

Macro to provide an option dependent on other options.

This macro presents an option to the user only if a set of other
conditions are true.

.. command:: cmake_dependent_option

  .. code-block:: cmake

    cmake_dependent_option(<option> "<help_text>" <value> <depends> <force>)

  Makes ``<option>`` available to the user if the
  :ref:`semicolon-separated list <CMake Language Lists>` of conditions in
  ``<depends>`` are all true.  Otherwise, a local variable named ``<option>``
  is set to ``<force>``.

  When ``<option>`` is available, the given ``<help_text>`` and initial
  ``<value>`` are used. Otherwise, any value set by the user is preserved for
  when ``<depends>`` is satisfied in the future.

  Note that the ``<option>`` variable only has a value which satisfies the
  ``<depends>`` condition within the scope of the caller because it is a local
  variable.

Example invocation:

.. code-block:: cmake

  cmake_dependent_option(USE_FOO "Use Foo" ON "USE_BAR;NOT USE_ZOT" OFF)

If ``USE_BAR`` is true and ``USE_ZOT`` is false, this provides an option called
``USE_FOO`` that defaults to ON. Otherwise, it sets ``USE_FOO`` to OFF and
hides the option from the user. If the status of ``USE_BAR`` or ``USE_ZOT``
ever changes, any value for the ``USE_FOO`` option is saved so that when the
option is re-enabled it retains its old value.

.. versionadded:: 3.22

  Full :ref:`Condition Syntax` is now supported.  See policy :policy:`CMP0127`.

#]=======================================================================]

macro(CMAKE_DEPENDENT_OPTION option doc default depends force)
  cmake_policy(GET CMP0127 _CDO_CMP0127
    PARENT_SCOPE # undocumented, do not use outside of CMake
    )
  if(${option}_ISSET MATCHES "^${option}_ISSET$")
    set(${option}_AVAILABLE 1)
    if("x${_CDO_CMP0127}x" STREQUAL "xNEWx")
      foreach(d ${depends})
        cmake_language(EVAL CODE "
          if (${d})
          else()
            set(${option}_AVAILABLE 0)
          endif()"
        )
      endforeach()
    else()
      foreach(d ${depends})
        string(REGEX REPLACE " +" ";" CMAKE_DEPENDENT_OPTION_DEP "${d}")
        if(${CMAKE_DEPENDENT_OPTION_DEP})
        else()
          set(${option}_AVAILABLE 0)
        endif()
      endforeach()
    endif()
    if(${option}_AVAILABLE)
      option(${option} "${doc}" "${default}")
      set(${option} "${${option}}" CACHE BOOL "${doc}" FORCE)
    else()
      if(${option} MATCHES "^${option}$")
      else()
        set(${option} "${${option}}" CACHE INTERNAL "${doc}")
      endif()
      set(${option} ${force})
    endif()
  else()
    set(${option} "${${option}_ISSET}")
  endif()
  if("x${_CDO_CMP0127}x" STREQUAL "xx" AND "x${depends}x" MATCHES "[^A-Za-z0-9_.; ]")
    cmake_policy(GET_WARNING CMP0127 _CDO_CMP0127_WARNING)
    message(AUTHOR_WARNING "${_CDO_CMP0127_WARNING}")
  endif()
  unset(_CDO_CMP0127)
endmacro()
