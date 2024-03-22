CMAKE_<LANG>_USING_LINKER_MODE
------------------------------

.. versionadded:: 3.29

This controls how the value of the :variable:`CMAKE_<LANG>_USING_LINKER_<TYPE>`
variable should be interpreted. The supported linker mode values are:

``FLAG``
  :variable:`CMAKE_<LANG>_USING_LINKER_<TYPE>` holds a
  :ref:`semicolon-separated list <CMake Language Lists>` of flags to be passed
  to the compiler frontend.  This is also the default behavior if
  ``CMAKE_<LANG>_USING_LINKER_MODE`` is not set.

``TOOL``
  :variable:`CMAKE_<LANG>_USING_LINKER_<TYPE>` holds the path to the linker
  tool.
