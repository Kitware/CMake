TARGET_LINKER_IMPORT_FILE_PREFIX
--------------------------------

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_LINKER_IMPORT_FILE_PREFIX:tgt>

.. versionadded:: 3.27

Prefix of the import file used to link target ``tgt``.

See also the :prop_tgt:`IMPORT_PREFIX` target property.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
