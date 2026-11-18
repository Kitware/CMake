VERSION_LESS_EQUAL
------------------

.. versionadded:: 3.7

.. |genex-family| replace:: :ref:`genex-family-version-comparisons`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<VERSION_LESS_EQUAL:v1,v2>

``1`` if ``v1`` is a version less than or equal to ``v2``, else ``0``.
