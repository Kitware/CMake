TARGET_LINKER_IMPORT_FILE_DIR
-----------------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_LINKER_IMPORT_FILE_DIR:tgt>

.. versionadded:: 3.27

Directory of the import file used to link target ``tgt``.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
