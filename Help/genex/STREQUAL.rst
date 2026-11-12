STREQUAL
--------

.. |genex-family| replace::
   :ref:`genex-family-string-comparisons-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<STREQUAL:string1,string2>

``1`` if ``string1`` and ``string2`` are lexicographically equal, else ``0``.
