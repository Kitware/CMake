TARGET_LINKER_LIBRARY_FILE_SUFFIX
---------------------------------

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_LINKER_LIBRARY_FILE_SUFFIX:tgt>

.. versionadded:: 3.27

Suffix of the library file used to link target ``tgt``.

The suffix corresponds to the file extension (such as ".a" or ".dylib").

See also the :prop_tgt:`SUFFIX` target property.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
