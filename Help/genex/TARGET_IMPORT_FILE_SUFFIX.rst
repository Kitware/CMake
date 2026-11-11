TARGET_IMPORT_FILE_SUFFIX
-------------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_IMPORT_FILE_SUFFIX:tgt>

.. versionadded:: 3.27

Suffix of the import file of the target ``tgt``.

The suffix corresponds to the file extension (such as ``.lib`` or ``.tbd``).

See also the :prop_tgt:`IMPORT_SUFFIX` target property.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
