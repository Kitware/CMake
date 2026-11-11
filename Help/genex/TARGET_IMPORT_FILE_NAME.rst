TARGET_IMPORT_FILE_NAME
-----------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_IMPORT_FILE_NAME:tgt>

.. versionadded:: 3.27

Name of the import file of the target ``tgt``.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
