TARGET_LINKER_FILE_PREFIX
-------------------------

.. versionadded:: 3.15

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<TARGET_LINKER_FILE_PREFIX:tgt>

Prefix of file used to link target ``tgt``.

See also the :prop_tgt:`PREFIX` and :prop_tgt:`IMPORT_PREFIX` target properties.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
