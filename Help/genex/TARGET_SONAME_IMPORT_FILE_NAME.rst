TARGET_SONAME_IMPORT_FILE_NAME
------------------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_SONAME_IMPORT_FILE_NAME:tgt>

.. versionadded:: 3.27

Name of the import file with soname (``.3.tbd``).

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
