STRGREATER
----------

.. versionadded:: 4.3

.. |genex-family| replace::
   :ref:`genex-family-string-comparisons-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<STRGREATER:string1,string2>

``1`` if ``string1`` is lexicographically greater than ``string2``, else ``0``.
