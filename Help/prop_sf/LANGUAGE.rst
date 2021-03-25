LANGUAGE
--------

Specify the programming language in which a source file is written.

A property that can be set to indicate what programming language the
source file is.  If it is not set the language is determined based on
the file extension.  Typical values are ``CXX`` (i.e.  C++), ``C``,
``CSharp``, ``CUDA``, ``Fortran``, ``ISPC``, and ``ASM``.  Setting this
property for a file means this file will be compiled.  Do not set this
for headers or files that should not be compiled.

.. versionchanged:: 3.20
  Setting this property causes the source file to be compiled as the
  specified language, using explicit flags if possible.  Previously it
  only caused the specified language's compiler to be used.
  See policy :policy:`CMP0119`.
