LANGUAGE
--------

Specify the language used to compile a source file.

This property selects the language whose compiler and compilation rules
are used for the source file.  If it is not set, the language is determined
from the file extension.  Typical values are ``CXX`` (i.e. C++), ``C``,
``CSharp``, ``CUDA``, ``Fortran``, ``HIP``, ``ISPC``, and ``ASM``.  Setting
this property for a file means this file will be compiled, unless
:prop_sf:`HEADER_FILE_ONLY` is set.

.. versionadded:: 4.5
  The :prop_fs:`LANGUAGE` property of a ``SOURCES`` file set takes precedence
  over this property when compiling a member of that set for a target.
  This target-specific selection does not modify the source file property.

.. versionchanged:: 3.20
  Setting this property causes the source file to be compiled as the
  specified language, using explicit flags if possible.  Previously it
  only caused the specified language's compiler to be used.
  See policy :policy:`CMP0119`.
