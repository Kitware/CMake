TARGET_FILE_NAME
----------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_FILE_NAME:tgt>

The ``tgt`` filename.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on (see policy :policy:`CMP0112`).
