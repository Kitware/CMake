CXX_MODULE_HEADER_UNIT_DIRS_<NAME>
----------------------------------

.. versionadded:: 3.25

.. note ::

  Experimental. Gated by ``CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API``

Semicolon-separated list of base directories of the target's ``<NAME>`` C++
module header set, which has the set type ``CXX_MODULE_HEADER_UNITS``. The
property supports
:manual:`generator expressions <cmake-generator-expressions(7)>`.

This property is normally only set by :command:`target_sources(FILE_SET)`
rather than being manipulated directly.

See :prop_tgt:`CXX_MODULE_HEADER_UNIT_DIRS` for the list of base directories
in the default C++ module header set. See
:prop_tgt:`CXX_MODULE_HEADER_UNIT_SETS` for the file set names of all C++
module header sets.
