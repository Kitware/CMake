TARGET_LINKER_LIBRARY_FILE_PREFIX
---------------------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_LINKER_LIBRARY_FILE_PREFIX:tgt>

.. versionadded:: 3.27

Prefix of the library file used to link target ``tgt``.

See also the :prop_tgt:`PREFIX` target property.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
