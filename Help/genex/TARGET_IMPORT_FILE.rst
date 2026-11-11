TARGET_IMPORT_FILE
------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_IMPORT_FILE:tgt>

.. versionadded:: 3.27

Full path to the linker import file. On DLL platforms, it would be the ``.lib``
file. For executables on AIX, and for shared libraries on macOS, it could be,
respectively, the ``.imp`` or ``.tbd`` import file, depending on the value of
the :prop_tgt:`ENABLE_EXPORTS` property.

This expands to an empty string when there is no import file associated with the
target.
