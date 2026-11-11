TARGET_LINKER_FILE_NAME
-----------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_LINKER_FILE_NAME:tgt>

Name of file used to link target ``tgt``.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on (see policy :policy:`CMP0112`).
