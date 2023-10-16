CMAKE_<LANG>_USING_LINKER_MODE
------------------------------

.. versionadded:: 3.29

This variable specify what is the type of data stored in variable
 :variable:`CMAKE_<LANG>_USING_LINKER_<TYPE>`. There are two possible values:

``FLAG``
  :variable:`CMAKE_<LANG>_USING_LINKER_<TYPE>` holds compiler flags. This is
  the default.

``TOOL``
  :variable:`CMAKE_<LANG>_USING_LINKER_<TYPE>` holds the path to the linker
  tool.
