COMPILE_LANG_AND_ID
-------------------

.. |genex-family| replace::
   :ref:`genex-family-compiler-language-id-and-frontend-variant`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<COMPILE_LANG_AND_ID:language,compiler_ids>

.. versionadded:: 3.15

``1`` when the language used for compilation unit matches ``language`` and
CMake's compiler id of the ``language`` compiler matches any one of the
comma-separated entries in ``compiler_ids``, otherwise ``0``. This expression is
a short form for the combination of ``$<COMPILE_LANGUAGE:language>`` and
``$<LANG_COMPILER_ID:compiler_ids>``. This expression may be used to specify
compile options, compile definitions, and include directories for source files
of a particular language and compiler combination in a target. For example:

.. code-block:: cmake

  add_executable(myapp main.cpp foo.c bar.cpp zot.cu)
  target_compile_definitions(myapp
    PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:COMPILING_CXX_WITH_CLANG>
            $<$<COMPILE_LANG_AND_ID:CXX,Intel>:COMPILING_CXX_WITH_INTEL>
            $<$<COMPILE_LANG_AND_ID:C,Clang>:COMPILING_C_WITH_CLANG>
  )

This specifies the use of different compile definitions based on both the
compiler id and compilation language. This example will have a
``COMPILING_CXX_WITH_CLANG`` compile definition when Clang is the CXX compiler,
and ``COMPILING_CXX_WITH_INTEL`` when Intel is the CXX compiler. Likewise, when
the C compiler is Clang, it will only see the ``COMPILING_C_WITH_CLANG``
definition.

Without the ``COMPILE_LANG_AND_ID`` generator expression, the same logic would
be expressed as:

.. code-block:: cmake

  target_compile_definitions(myapp
    PRIVATE $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:AppleClang,Clang>>:COMPILING_CXX_WITH_CLANG>
            $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:Intel>>:COMPILING_CXX_WITH_INTEL>
            $<$<AND:$<COMPILE_LANGUAGE:C>,$<C_COMPILER_ID:Clang>>:COMPILING_C_WITH_CLANG>
  )
