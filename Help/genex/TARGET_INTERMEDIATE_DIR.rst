TARGET_INTERMEDIATE_DIR
-----------------------

.. versionadded:: 4.2

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<TARGET_INTERMEDIATE_DIR:tgt>

The full path to the directory where intermediate target files, such as object
and dependency files, are stored.
