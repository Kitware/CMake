TARGET_LINKER_IMPORT_FILE_NAME
------------------------------

.. versionadded:: 3.27

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<TARGET_LINKER_IMPORT_FILE_NAME:tgt>

Name of the import file used to link target ``tgt``.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
