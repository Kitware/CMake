STRING:ASCII
------------

.. versionadded:: 4.3

.. |genex-family| replace::
   :ref:`genex-family-string-generating-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<STRING:ASCII,number[,number]...>

Convert all numbers, in the range 1-255, into corresponding ASCII characters.
Any number outside this range will raise an error.
