CXX_MODULE_HEADER_UNIT_SET
--------------------------

.. versionadded:: 3.25

.. note ::

  Experimental. Gated by ``CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API``

Semicolon-separated list of files in the target's default C++ module header
set, (i.e. the file set with name and type ``CXX_MODULE_HEADER_UNITS``). If
any of the paths are relative, they are computed relative to the target's
source directory. The property supports
:manual:`generator expressions <cmake-generator-expressions(7)>`.

This property is normally only set by :command:`target_sources(FILE_SET)`
rather than being manipulated directly.

See :prop_tgt:`CXX_MODULE_HEADER_UNIT_SET_<NAME>` for the list of files in
other C++ module header sets.
