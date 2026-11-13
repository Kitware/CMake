TARGET_LINKER_LIBRARY_FILE
--------------------------

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_LINKER_LIBRARY_FILE:tgt>

.. versionadded:: 3.27

File used when linking o the ``tgt`` target is done using directly the library,
and not an import file. This will usually be the library that ``tgt`` represents
(``.a``, ``.so``, ``.dylib``). So, on DLL platforms, it will be an empty string.
