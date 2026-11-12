VERSION_GREATER_EQUAL
---------------------

.. |genex-family| replace:: :ref:`genex-family-version-comparisons`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<VERSION_GREATER_EQUAL:v1,v2>

.. versionadded:: 3.7

``1`` if ``v1`` is a version greater than or equal to ``v2``, else ``0``.
