VERSION_GREATER
---------------

.. |genex-family| replace:: :ref:`genex-family-version-comparisons`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<VERSION_GREATER:v1,v2>

``1`` if ``v1`` is a version greater than ``v2``, else ``0``.
