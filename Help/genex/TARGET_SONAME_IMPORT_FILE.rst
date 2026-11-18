TARGET_SONAME_IMPORT_FILE
-------------------------

.. versionadded:: 3.27

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<TARGET_SONAME_IMPORT_FILE:tgt>

Import file with soname (``.3.tbd``) where ``tgt`` is the name of a target.
