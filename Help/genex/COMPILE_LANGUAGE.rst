COMPILE_LANGUAGE
----------------

.. genex:: $<COMPILE_LANGUAGE:languages>
   :target: COMPILE_LANGUAGE:languages

.. |genex-family| replace::
   :ref:`genex-family-compiler-language-id-and-frontend-variant`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<COMPILE_LANGUAGE>

.. versionadded:: 3.3

The compile language of source files when evaluating compile options. See the
related boolean expression :genex:`$<COMPILE_LANGUAGE:languages>
<COMPILE_LANGUAGE:languages>` for notes about the portability of this generator
expression.

Syntax
^^^^^^

.. code-block:: cmake

  $<COMPILE_LANGUAGE:languages>

.. versionadded:: 3.3

.. versionchanged:: 3.15
  Multiple languages can be specified for ``languages``.
  CMake 3.14 and earlier only accepted a single language.

``1`` when the language used for compilation unit matches any of the
comma-separated entries in ``languages``, otherwise ``0``. This expression may
be used to specify compile options, compile definitions, and include directories
for source files of a particular language in a target. For example:

.. code-block:: cmake

  add_executable(myapp main.cpp foo.c bar.cpp zot.cu)
  target_compile_options(myapp
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
  )
  target_compile_definitions(myapp
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:COMPILING_CXX>
            $<$<COMPILE_LANGUAGE:CUDA>:COMPILING_CUDA>
  )
  target_include_directories(myapp
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,CUDA>:/opt/foo/headers>
  )

This specifies the use of the ``-fno-exceptions`` compile option,
``COMPILING_CXX`` compile definition, and ``cxx_headers`` include directory for
C++ only (compiler id checks elided).  It also specifies a ``COMPILING_CUDA``
compile definition for CUDA.

Note that with :ref:`Visual Studio Generators` and :generator:`Xcode` there is
no way to represent target-wide compile definitions or include directories
separately for ``C`` and ``CXX`` languages. Also, with :ref:`Visual Studio
Generators` there is no way to represent target-wide flags separately for ``C``
and ``CXX`` languages.  Under these generators, expressions for both C and C++
sources will be evaluated using ``CXX`` if there are any C++ sources and
otherwise using ``C``. A workaround is to create separate libraries for each
source file language instead:

.. code-block:: cmake

  add_library(myapp_c foo.c)
  add_library(myapp_cxx bar.cpp)
  target_compile_options(myapp_cxx PUBLIC -fno-exceptions)
  add_executable(myapp main.cpp)
  target_link_libraries(myapp myapp_c myapp_cxx)
