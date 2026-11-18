STRGREATER_EQUAL
----------------

.. versionadded:: 4.3

.. |genex-family| replace::
   :ref:`genex-family-string-comparisons-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<STRGREATER_EQUAL:string1,string2>

``1`` if ``string1`` is lexicographically greater than or equal to ``string2``,
else ``0``.
