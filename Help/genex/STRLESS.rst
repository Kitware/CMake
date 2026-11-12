STRLESS
-------

.. |genex-family| replace::
   :ref:`genex-family-string-comparisons-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<STRLESS:string1,string2>

.. versionadded:: 4.3

``1`` if ``string1`` is lexicographically less than ``string2``, else ``0``.
