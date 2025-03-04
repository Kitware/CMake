# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckPIESupported
-----------------

.. versionadded:: 3.14

Check whether the linker supports Position Independent Code (PIE) or No
Position Independent Code (NO_PIE) for executables.
Use this to ensure that the :prop_tgt:`POSITION_INDEPENDENT_CODE` target
property for executables will be honored at link time.

.. command:: check_pie_supported

  .. code-block:: cmake

    check_pie_supported([OUTPUT_VARIABLE <output>]
                        [LANGUAGES <lang>...])

  Options are:

  ``OUTPUT_VARIABLE <output>``
    Set ``<output>`` variable with details about any error. If the check is
    bypassed because it uses cached results from a previous call, the output
    will be empty even if errors were present in the previous call.

  ``LANGUAGES <lang>...``
    Check the linkers used for each of the specified languages.
    If this option is not provided, the command checks all enabled languages.

    ``C``, ``CXX``, ``Fortran`` are supported.

    .. versionadded:: 3.23

      ``OBJC``, ``OBJCXX``, ``CUDA``, and ``HIP`` are supported.

It makes no sense to use this module when :policy:`CMP0083` is set to ``OLD``,
so the command will return an error in this case.  See policy :policy:`CMP0083`
for details.

Variables
^^^^^^^^^

For each language checked, two boolean cache variables are defined.

 ``CMAKE_<lang>_LINK_PIE_SUPPORTED``
   Set to true if ``PIE`` is supported by the linker and false otherwise.
 ``CMAKE_<lang>_LINK_NO_PIE_SUPPORTED``
   Set to true if ``NO_PIE`` is supported by the linker and false otherwise.

Examples
^^^^^^^^

.. code-block:: cmake

  check_pie_supported()
  set_property(TARGET foo PROPERTY POSITION_INDEPENDENT_CODE TRUE)

.. code-block:: cmake

  # Retrieve any error message.
  check_pie_supported(OUTPUT_VARIABLE output LANGUAGES C)
  set_property(TARGET foo PROPERTY POSITION_INDEPENDENT_CODE TRUE)
  if(NOT CMAKE_C_LINK_PIE_SUPPORTED)
    message(WARNING "PIE is not supported at link time: ${output}.\n"
                    "PIE link options will not be passed to linker.")
  endif()

#]=======================================================================]


include (Internal/CheckLinkerFlag)

function (check_pie_supported)
  cmake_policy(GET CMP0083 cmp0083)

  if (NOT cmp0083)
    message(FATAL_ERROR "check_pie_supported: Policy CMP0083 is not set")
  endif()

  if(cmp0083 STREQUAL "OLD")
    message(FATAL_ERROR "check_pie_supported: Policy CMP0083 set to OLD")
  endif()

  set(optional)
  set(one OUTPUT_VARIABLE)
  set(multiple LANGUAGES)

  cmake_parse_arguments(CHECK_PIE "${optional}" "${one}" "${multiple}" "${ARGN}")
  if(CHECK_PIE_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "check_pie_supported: Unparsed arguments: ${CHECK_PIE_UNPARSED_ARGUMENTS}")
  endif()

  if (CHECK_PIE_LANGUAGES)
    set (unsupported_languages "${CHECK_PIE_LANGUAGES}")
    list (REMOVE_ITEM unsupported_languages "C" "CXX" "OBJC" "OBJCXX" "Fortran" "CUDA" "HIP")
    if(unsupported_languages)
      message(FATAL_ERROR "check_pie_supported: language(s) '${unsupported_languages}' not supported")
    endif()
  else()
    # User did not set any languages, use defaults
    get_property (enabled_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
    if (NOT enabled_languages)
      return()
    endif()

    list (FILTER enabled_languages INCLUDE REGEX "^(C|CXX|OBJC|OBJCXX|Fortran|CUDA|HIP)$")
    if (NOT enabled_languages)
      return()
    endif()

    set (CHECK_PIE_LANGUAGES ${enabled_languages})
  endif()

  set(CMAKE_REQUIRED_QUIET TRUE)
  set (outputs)

  foreach(lang IN LISTS CHECK_PIE_LANGUAGES)
    if(_CMAKE_${lang}_PIE_MAY_BE_SUPPORTED_BY_LINKER)
      if(NOT DEFINED CMAKE_${lang}_LINK_PIE_SUPPORTED)
        # ensure PIE compile flags are also used
        list(JOIN CMAKE_${lang}_COMPILE_OPTIONS_PIE " " CMAKE_REQUIRED_FLAGS)
        cmake_check_linker_flag(${lang}
                                "${CMAKE_${lang}_LINK_OPTIONS_PIE}"
                                CMAKE_${lang}_LINK_PIE_SUPPORTED
                                OUTPUT_VARIABLE output)
        if (NOT CMAKE_${lang}_LINK_PIE_SUPPORTED)
          string (APPEND outputs "PIE (${lang}): ${output}\n")
        endif()
        unset(CMAKE_REQUIRED_FLAGS)
      endif()

      if(NOT DEFINED CMAKE_${lang}_LINK_NO_PIE_SUPPORTED)
        cmake_check_linker_flag(${lang}
                                "${CMAKE_${lang}_LINK_OPTIONS_NO_PIE}"
                                CMAKE_${lang}_LINK_NO_PIE_SUPPORTED
                                OUTPUT_VARIABLE output)
        if (NOT CMAKE_${lang}_LINK_NO_PIE_SUPPORTED)
          string (APPEND outputs "NO_PIE (${lang}): ${output}\n")
        endif()
      endif()
    else()
      # no support at link time. Set cache variables to NO
      set(CMAKE_${lang}_LINK_PIE_SUPPORTED NO CACHE INTERNAL "PIE (${lang})")
      set(CMAKE_${lang}_LINK_NO_PIE_SUPPORTED NO CACHE INTERNAL "NO_PIE (${lang})")
      string (APPEND outputs "PIE and NO_PIE are not supported by linker for ${lang}\n")
    endif()
  endforeach()

  if (CHECK_PIE_OUTPUT_VARIABLE)
    set (${CHECK_PIE_OUTPUT_VARIABLE} "${outputs}" PARENT_SCOPE)
  endif()
endfunction()
