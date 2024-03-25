CMAKE_<LANG>_USING_LINKER_MODE
------------------------------

.. versionadded:: 3.29

This controls how the value of the :variable:`CMAKE_<LANG>_USING_LINKER_<TYPE>`
variable should be interpreted. The supported linker mode values are:

``FLAG``
  :variable:`CMAKE_<LANG>_USING_LINKER_<TYPE>` holds compiler flags. This is
  the default.

``TOOL``
  :variable:`CMAKE_<LANG>_USING_LINKER_<TYPE>` holds the path to the linker
  tool.
