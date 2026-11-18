TARGET_PDB_FILE_DIR
-------------------

.. versionadded:: 3.1

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<TARGET_PDB_FILE_DIR:tgt>

Directory of the linker generated program database file (.pdb).

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on (see policy :policy:`CMP0112`).
