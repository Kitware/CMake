TARGET_SONAME_FILE_DIR
----------------------

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_SONAME_FILE_DIR:tgt>

Directory of file with soname (``.so.3``).

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on (see policy :policy:`CMP0112`).
