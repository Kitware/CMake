TARGET_LINKER_LIBRARY_FILE_DIR
------------------------------

.. versionadded:: 3.27

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<TARGET_LINKER_LIBRARY_FILE_DIR:tgt>

Directory of the library file used to link target ``tgt``.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
