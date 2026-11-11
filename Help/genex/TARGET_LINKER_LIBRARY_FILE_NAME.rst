TARGET_LINKER_LIBRARY_FILE_NAME
-------------------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_LINKER_LIBRARY_FILE_NAME:tgt>

.. versionadded:: 3.27

Name of the library file used to link target ``tgt``.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
