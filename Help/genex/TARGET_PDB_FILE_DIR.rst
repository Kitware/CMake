TARGET_PDB_FILE_DIR
-------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_PDB_FILE_DIR:tgt>

.. versionadded:: 3.1

Directory of the linker generated program database file (.pdb).

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on (see policy :policy:`CMP0112`).
