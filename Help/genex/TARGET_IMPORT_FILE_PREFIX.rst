TARGET_IMPORT_FILE_PREFIX
-------------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_IMPORT_FILE_PREFIX:tgt>

.. versionadded:: 3.27

Prefix of the import file of the target ``tgt``.

See also the :prop_tgt:`IMPORT_PREFIX` target property.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
