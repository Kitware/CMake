TARGET_FILE_DIR
---------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_FILE_DIR:tgt>

Directory of the ``tgt`` binary file.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on (see policy :policy:`CMP0112`).
