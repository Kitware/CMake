VERSION_EQUAL
-------------

.. |genex-family| replace:: :ref:`genex-family-version-comparisons`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<VERSION_EQUAL:v1,v2>

``1`` if ``v1`` is the same version as ``v2``, else ``0``.
