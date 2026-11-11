TARGET_SONAME_FILE_NAME
-----------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_SONAME_FILE_NAME:tgt>

Name of file with soname (``.so.3``).

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on (see policy :policy:`CMP0112`).
