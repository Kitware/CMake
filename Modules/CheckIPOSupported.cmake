# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
CheckIPOSupported
-----------------

.. versionadded:: 3.9

Check whether the compiler supports an interprocedural optimization (IPO/LTO).
Use this before enabling the :prop_tgt:`INTERPROCEDURAL_OPTIMIZATION` target
property.

.. command:: check_ipo_supported

  .. code-block:: cmake

    check_ipo_supported([RESULT <result>] [OUTPUT <output>]
                        [LANGUAGES <lang>...])

  Options are:

  ``RESULT <result>``
    Set ``<result>`` variable to ``YES`` if IPO is supported by the
    compiler and ``NO`` otherwise.  If this option is not given then
    the command will issue a fatal error if IPO is not supported.
  ``OUTPUT <output>``
    Set ``<output>`` variable with details about any error.
  ``LANGUAGES <lang>...``
    Specify languages whose compilers to check.

    The following languages are supported:

    * ``C``

    * ``CXX``

    * ``CUDA``

      .. versionadded:: 3.25

    * ``Fortran``

    If this option is not given, the default languages are picked from
    the current :prop_gbl:`ENABLED_LANGUAGES` global property.

.. note::

  To use ``check_ipo_supported()``, policy :policy:`CMP0069` must be set to
  ``NEW``; otherwise, a fatal error will occur.

.. versionadded:: 3.13
  Support for Visual Studio generators.

.. versionadded:: 3.24
  The check uses the caller's :variable:`CMAKE_<LANG>_FLAGS`
  and :variable:`CMAKE_<LANG>_FLAGS_<CONFIG>` values.
  See policy :policy:`CMP0138`.

Examples
^^^^^^^^

.. code-block:: cmake

  check_ipo_supported() # fatal error if IPO is not supported
  set_property(TARGET foo PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

.. code-block:: cmake

  # Optional IPO. Do not use IPO if it's not supported by compiler.
  check_ipo_supported(RESULT result OUTPUT output)
  if(result)
    set_property(TARGET foo PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  else()
    message(WARNING "IPO is not supported: ${output}")
  endif()

#]=======================================================================]

# X_RESULT - name of the final result variable
# X_OUTPUT - name of the variable with information about error
macro(_ipo_not_supported output)
  if(NOT X_RESULT)
    message(FATAL_ERROR "IPO is not supported (${output}).")
  endif()

  set("${X_RESULT}" NO PARENT_SCOPE)
  if(X_OUTPUT)
    set("${X_OUTPUT}" "${output}" PARENT_SCOPE)
  endif()
endmacro()

# Run IPO/LTO test
macro(_ipo_run_language_check language)
  set(_C_ext "c")
  set(_CXX_ext "cpp")
  set(_Fortran_ext "f")
  string(COMPARE EQUAL "${language}" "CUDA" is_cuda)

  set(ext ${_${language}_ext})
  if(NOT "${ext}" STREQUAL "")
    set(copy_sources foo.${ext} main.${ext})
  elseif(is_cuda)
    if(_CMAKE_CUDA_IPO_SUPPORTED_BY_CMAKE)
      set("${X_RESULT}" YES PARENT_SCOPE)
    endif()
    return()
  else()
    message(FATAL_ERROR "Language not supported")
  endif()

  set(testdir "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/_CMakeLTOTest-${language}")

  file(REMOVE_RECURSE "${testdir}")
  file(MAKE_DIRECTORY "${testdir}")

  set(bindir "${testdir}/bin")
  set(srcdir "${testdir}/src")

  file(MAKE_DIRECTORY "${bindir}")
  file(MAKE_DIRECTORY "${srcdir}")

  set(TRY_COMPILE_PROJECT_NAME "lto-test")

  set(try_compile_src "${CMAKE_ROOT}/Modules/CheckIPOSupported")

  # Use:
  # * TRY_COMPILE_PROJECT_NAME
  # * CMAKE_VERSION
  configure_file(
      "${try_compile_src}/CMakeLists-${language}.txt.in"
      "${srcdir}/CMakeLists.txt"
      @ONLY
  )

  foreach(x ${copy_sources})
    configure_file(
        "${try_compile_src}/${x}"
        "${srcdir}/${x}"
        COPYONLY
    )
  endforeach()

  if(ipo_CMP0138 STREQUAL "NEW")
    set(CMAKE_TRY_COMPILE_CONFIGURATION Debug)
    set(_CMAKE_LANG_FLAGS
      "-DCMAKE_${language}_FLAGS:STRING=${CMAKE_${language}_FLAGS}"
      "-DCMAKE_${language}_FLAGS_DEBUG:STRING=${CMAKE_${language}_FLAGS_DEBUG}"
      )
  else()
    set(_CMAKE_LANG_FLAGS "")
  endif()

  try_compile(
      _IPO_LANGUAGE_CHECK_RESULT
      PROJECT "${TRY_COMPILE_PROJECT_NAME}"
      SOURCE_DIR "${srcdir}"
      BINARY_DIR "${bindir}"
      CMAKE_FLAGS
      "-DCMAKE_VERBOSE_MAKEFILE=ON"
      "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON"
      ${_CMAKE_LANG_FLAGS}
      OUTPUT_VARIABLE output
  )
  set(_IPO_LANGUAGE_CHECK_RESULT "${_IPO_LANGUAGE_CHECK_RESULT}")
  unset(_IPO_LANGUAGE_CHECK_RESULT CACHE)

  if(NOT _IPO_LANGUAGE_CHECK_RESULT)
    _ipo_not_supported("check failed to compile")
    if(X_OUTPUT)
      set("${X_OUTPUT}" "${output}" PARENT_SCOPE)
    endif()
    return()
  endif()
endmacro()

function(check_ipo_supported)
  cmake_policy(GET CMP0069 x)

  string(COMPARE EQUAL "${x}" "" not_set)
  if(not_set)
    message(FATAL_ERROR "Policy CMP0069 is not set")
  endif()

  string(COMPARE EQUAL "${x}" "OLD" is_old)
  if(is_old)
    message(FATAL_ERROR "Policy CMP0069 set to OLD")
  endif()

  # Save policy setting for condition in _ipo_run_language_check.
  cmake_policy(GET CMP0138 ipo_CMP0138
    PARENT_SCOPE # undocumented, do not use outside of CMake
    )

  set(optional)
  set(one RESULT OUTPUT)
  set(multiple LANGUAGES)

  # Introduce:
  # * X_RESULT
  # * X_OUTPUT
  # * X_LANGUAGES
  cmake_parse_arguments(X "${optional}" "${one}" "${multiple}" "${ARGV}")

  string(COMPARE NOTEQUAL "${X_UNPARSED_ARGUMENTS}" "" has_unparsed)
  if(has_unparsed)
    message(FATAL_ERROR "Unparsed arguments: ${X_UNPARSED_ARGUMENTS}")
  endif()

  string(COMPARE EQUAL "${X_LANGUAGES}" "" no_languages)
  if(no_languages)
    # User did not set any languages, use defaults
    get_property(enabled_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
    string(COMPARE EQUAL "${enabled_languages}" "" no_languages)
    if(no_languages)
      _ipo_not_supported(
          "no languages found in ENABLED_LANGUAGES global property"
      )
      return()
    endif()

    set(languages "")
    list(FIND enabled_languages "CXX" result)
    if(NOT result EQUAL -1)
      list(APPEND languages "CXX")
    endif()

    list(FIND enabled_languages "C" result)
    if(NOT result EQUAL -1)
      list(APPEND languages "C")
    endif()

    list(FIND enabled_languages "CUDA" result)
    if(NOT result EQUAL -1)
      list(APPEND languages "CUDA")
    endif()

    list(FIND enabled_languages "Fortran" result)
    if(NOT result EQUAL -1)
      list(APPEND languages "Fortran")
    endif()

    string(COMPARE EQUAL "${languages}" "" no_languages)
    if(no_languages)
      _ipo_not_supported(
          "no C/CXX/CUDA/Fortran languages found in ENABLED_LANGUAGES global property"
      )
      return()
    endif()
  else()
    set(languages "${X_LANGUAGES}")

    set(unsupported_languages "${languages}")
    list(REMOVE_ITEM unsupported_languages "C" "CXX" "CUDA" "Fortran")
    string(COMPARE NOTEQUAL "${unsupported_languages}" "" has_unsupported)
    if(has_unsupported)
      _ipo_not_supported(
          "language(s) '${unsupported_languages}' not supported"
      )
      return()
    endif()
  endif()

  foreach(lang ${languages})
    if(NOT _CMAKE_${lang}_IPO_SUPPORTED_BY_CMAKE)
      _ipo_not_supported("CMake doesn't support IPO for current ${lang} compiler")
      return()
    endif()

    if(NOT _CMAKE_${lang}_IPO_MAY_BE_SUPPORTED_BY_COMPILER)
      _ipo_not_supported("${lang} compiler doesn't support IPO")
      return()
    endif()
  endforeach()

  foreach(x ${languages})
    _ipo_run_language_check(${x})
  endforeach()

  set("${X_RESULT}" YES PARENT_SCOPE)
endfunction()
