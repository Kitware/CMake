TARGET_LINKER_FILE
------------------

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_LINKER_FILE:tgt>

File used when linking to the ``tgt`` target.  This will usually be the library
that ``tgt`` represents (``.a``, ``.lib``, ``.so``), but for a shared library on
DLL platforms, it would be the ``.lib`` import library associated with the DLL.

.. versionadded:: 3.27
  On macOS, it could be the ``.tbd`` import file associated with the shared
  library, depending on the value of the :prop_tgt:`ENABLE_EXPORTS` property.

This generator expression is equivalent to
:genex:`$<TARGET_LINKER_LIBRARY_FILE>` or :genex:`$<TARGET_LINKER_IMPORT_FILE>`
generator expressions, depending on the characteristics of the target and the
platform.
