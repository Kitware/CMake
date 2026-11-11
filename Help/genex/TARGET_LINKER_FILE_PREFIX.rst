TARGET_LINKER_FILE_PREFIX
-------------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_LINKER_FILE_PREFIX:tgt>

.. versionadded:: 3.15

Prefix of file used to link target ``tgt``.

See also the :prop_tgt:`PREFIX` and :prop_tgt:`IMPORT_PREFIX` target properties.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
