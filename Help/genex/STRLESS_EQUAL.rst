STRLESS_EQUAL
-------------

.. |genex-family| replace::
   :ref:`genex-family-string-comparisons-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<STRLESS_EQUAL:string1,string2>

.. versionadded:: 4.3

``1`` if ``string1`` is lexicographically less than or equal to ``string2``,
else ``0``.
