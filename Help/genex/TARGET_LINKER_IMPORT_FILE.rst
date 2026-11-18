TARGET_LINKER_IMPORT_FILE
-------------------------

.. versionadded:: 3.27

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<TARGET_LINKER_IMPORT_FILE:tgt>

File used when linking to the ``tgt`` target is done using an import file.  This
will usually be the import file that ``tgt`` represents (``.lib``, ``.tbd``).
So, when no import file is involved in the link step, an empty string is
returned.
