TARGET_PDB_FILE_BASE_NAME
-------------------------

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_PDB_FILE_BASE_NAME:tgt[,POSTFIX:(INCLUDE|EXCLUDE)]>

.. versionadded:: 3.15

Base name of the linker generated program database file (.pdb) where ``tgt`` is
the name of a target.

.. versionadded:: 4.2
  The ``POSTFIX`` option can be used to control the inclusion or not
  of the :prop_tgt:`<CONFIG>_POSTFIX` target property value as part of the
  base name. The default is ``POSTFIX:INCLUDE``.

.. versionchanged:: 4.2
  The postfix, as specified by :prop_tgt:`DEBUG_POSTFIX` or
  :prop_tgt:`<CONFIG>_POSTFIX` target properties, is always included in the
  ``PDB`` base name, except if the ``POSTFIX`` option has the value
  ``EXCLUDE``.  See the policy :policy:`CMP0202`.

The base name corresponds to the target PDB file name (see
``$<TARGET_PDB_FILE_NAME:tgt>``) without prefix and suffix, and, optionally,
postfix.  For example, if the target file name is ``base_postfix.pdb``, the base
name is

  * ``base_postfix`` for ``$<TARGET_PDB_FILE_BASE_NAME:tgt>`` or
    ``$<TARGET_PDB_FILE_BASE_NAME:tgt,POSTFIX:INCLUDE>``.
  * ``base`` for ``$<TARGET_PDB_FILE_BASE_NAME:tgt,POSTFIX:EXCLUDE>``.

See also the :prop_tgt:`OUTPUT_NAME` and :prop_tgt:`PDB_NAME` target properties,
their configuration-specific variants :prop_tgt:`OUTPUT_NAME_<CONFIG>` and
:prop_tgt:`PDB_NAME_<CONFIG>`, and the :prop_tgt:`<CONFIG>_POSTFIX` and
:prop_tgt:`DEBUG_POSTFIX` target properties.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
