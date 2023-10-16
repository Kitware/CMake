LINKER_TYPE
-----------

.. versionadded:: 3.29

Specify which linker will be used for the link step. The property value may use
:manual:`generator expressions <cmake-generator-expressions(7)>`.

.. code-block:: cmake

  add_library(lib1 SHARED ...)
  set_property(TARGET lib1 PROPERTY LINKER_TYPE LLD)

This specifies that ``lib1`` should use linker type ``LLD`` for the link step.
The implementation details will be provided by the variable
:variable:`CMAKE_<LANG>_USING_LINKER_<TYPE>` with ``<TYPE>`` having the value
``LLD``.

This property is not supported on :generator:`Green Hills MULTI` and
:generator:`Visual Studio 9 2008` generators.

.. note::
  It is assumed that the linker specified is fully compatible with the standard
  one. CMake will not do any options translation.

.. include:: ../variable/LINKER_PREDEFINED_TYPES.txt
