TARGET_SONAME_IMPORT_FILE_DIR
-----------------------------

.. versionadded:: 3.27

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<TARGET_SONAME_IMPORT_FILE_DIR:tgt>

Directory of the import file with soname (``.3.tbd``).

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
