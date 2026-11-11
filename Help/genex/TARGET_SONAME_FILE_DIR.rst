TARGET_SONAME_FILE_DIR
----------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_SONAME_FILE_DIR:tgt>

Directory of file with soname (``.so.3``).

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on (see policy :policy:`CMP0112`).
