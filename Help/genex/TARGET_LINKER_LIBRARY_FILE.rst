TARGET_LINKER_LIBRARY_FILE
--------------------------

.. versionadded:: 3.27

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<TARGET_LINKER_LIBRARY_FILE:tgt>

File used when linking o the ``tgt`` target is done using directly the library,
and not an import file. This will usually be the library that ``tgt`` represents
(``.a``, ``.so``, ``.dylib``). So, on DLL platforms, it will be an empty string.
