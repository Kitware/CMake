LINK_LANG_AND_ID
----------------

Syntax
^^^^^^

.. code-block:: cmake

  $<LINK_LANG_AND_ID:language,compiler_ids>

.. versionadded:: 3.18

``1`` when the language used for link step matches ``language`` and the CMake's
compiler id of the language linker matches any one of the comma-separated
entries in ``compiler_ids``, otherwise ``0``. This expression is a short form
for the combination of ``$<LINK_LANGUAGE:language>`` and
``$<LANG_COMPILER_ID:compiler_ids>``. This expression may be used to specify
link libraries, link options, link directories and link dependencies of a
particular language and linker combination in a target. For example:

.. code-block:: cmake

  add_library(libC_Clang ...)
  add_library(libCXX_Clang ...)
  add_library(libC_Intel ...)
  add_library(libCXX_Intel ...)

  add_executable(myapp main.c)
  if (CXX_CONFIG)
    target_sources(myapp PRIVATE file.cxx)
  endif()
  target_link_libraries(myapp
    PRIVATE $<$<LINK_LANG_AND_ID:CXX,Clang,AppleClang>:libCXX_Clang>
            $<$<LINK_LANG_AND_ID:C,Clang,AppleClang>:libC_Clang>
            $<$<LINK_LANG_AND_ID:CXX,Intel>:libCXX_Intel>
            $<$<LINK_LANG_AND_ID:C,Intel>:libC_Intel>)

This specifies the use of different link libraries based on both the compiler id
and link language. This example will have target ``libCXX_Clang`` as link
dependency when ``Clang`` or ``AppleClang`` is the ``CXX`` linker, and
``libCXX_Intel`` when ``Intel`` is the ``CXX`` linker. Likewise when the ``C``
linker is ``Clang`` or ``AppleClang``, target ``libC_Clang`` will be added as
link dependency and ``libC_Intel`` when ``Intel`` is the ``C`` linker.

See :ref:`the note related to <Constraints LINK_LANGUAGE Generator Expression>`
``$<LINK_LANGUAGE:language>`` for constraints about the usage of this generator
expression.
