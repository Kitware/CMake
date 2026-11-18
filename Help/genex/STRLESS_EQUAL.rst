STRLESS_EQUAL
-------------

.. versionadded:: 4.3

.. |genex-family| replace::
   :ref:`genex-family-string-comparisons-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<STRLESS_EQUAL:string1,string2>

``1`` if ``string1`` is lexicographically less than or equal to ``string2``,
else ``0``.
