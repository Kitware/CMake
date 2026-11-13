TARGET_PDB_FILE
---------------

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_PDB_FILE:tgt>

.. versionadded:: 3.1

Full path to the linker generated program database file (.pdb) where ``tgt`` is
the name of a target.

.. versionchanged:: 4.2
  The postfix, as specified by :prop_tgt:`DEBUG_POSTFIX` or
  :prop_tgt:`<CONFIG>_POSTFIX` target properties, is always included in the
  ``PDB`` file name. See the policy :policy:`CMP0202`.

See also the :prop_tgt:`PDB_NAME` and :prop_tgt:`PDB_OUTPUT_DIRECTORY` target
properties and their configuration specific variants
:prop_tgt:`PDB_NAME_<CONFIG>` and :prop_tgt:`PDB_OUTPUT_DIRECTORY_<CONFIG>`.
