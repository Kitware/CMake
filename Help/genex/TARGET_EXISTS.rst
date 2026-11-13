TARGET_EXISTS
-------------

.. |genex-family| replace:: :ref:`genex-family-target-meta-data`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_EXISTS:tgt>

.. versionadded:: 3.12

``1`` if ``tgt`` exists as a CMake target, else ``0``.
