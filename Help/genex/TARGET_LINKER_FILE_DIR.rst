TARGET_LINKER_FILE_DIR
----------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_LINKER_FILE_DIR:tgt>

Directory of file used to link target ``tgt``.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on (see policy :policy:`CMP0112`).
