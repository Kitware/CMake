TARGET_LINKER_FILE_SUFFIX
-------------------------

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_LINKER_FILE_SUFFIX:tgt>

.. versionadded:: 3.15

Suffix of file used to link where ``tgt`` is the name of a target.

The suffix corresponds to the file extension (such as ".so" or ".lib").

See also the :prop_tgt:`SUFFIX` and :prop_tgt:`IMPORT_SUFFIX` target properties.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
