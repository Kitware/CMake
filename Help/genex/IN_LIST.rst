IN_LIST
-------

.. |genex-family| replace:: :ref:`genex-family-list-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<IN_LIST:string,list>

.. versionadded:: 3.12

``1`` if ``string`` is an item in the semicolon-separated ``list``, else ``0``.
It uses case-sensitive comparisons.
