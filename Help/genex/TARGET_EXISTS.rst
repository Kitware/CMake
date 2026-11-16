TARGET_EXISTS
-------------

.. versionadded:: 3.12

.. |genex-family| replace:: :ref:`genex-family-target-meta-data`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<TARGET_EXISTS:tgt>

``1`` if ``tgt`` exists as a CMake target, else ``0``.
