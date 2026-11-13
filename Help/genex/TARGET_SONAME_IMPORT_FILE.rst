TARGET_SONAME_IMPORT_FILE
-------------------------

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_SONAME_IMPORT_FILE:tgt>

.. versionadded:: 3.27

Import file with soname (``.3.tbd``) where ``tgt`` is the name of a target.
