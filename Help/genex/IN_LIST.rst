IN_LIST
-------

.. versionadded:: 3.12

.. |genex-family| replace:: :ref:`genex-family-list-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<IN_LIST:string,list>

``1`` if ``string`` is an item in the semicolon-separated ``list``, else ``0``.
It uses case-sensitive comparisons.
