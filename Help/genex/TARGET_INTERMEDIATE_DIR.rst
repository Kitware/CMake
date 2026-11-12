TARGET_INTERMEDIATE_DIR
-----------------------

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_INTERMEDIATE_DIR:tgt>

.. versionadded:: 4.2

The full path to the directory where intermediate target files, such as object
and dependency files, are stored.
