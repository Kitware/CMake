TARGET_PDB_FILE_NAME
--------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_PDB_FILE_NAME:tgt>

.. versionadded:: 3.1

Name of the linker generated program database file (.pdb).

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on (see policy :policy:`CMP0112`).
