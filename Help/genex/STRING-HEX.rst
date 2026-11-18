STRING:HEX
----------

.. versionadded:: 4.3

.. |genex-family| replace::
   :ref:`genex-family-string-transforming-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<STRING:HEX,string>

Convert each byte in the ``string`` to its hexadecimal representation. Letters
in the result (a through f) are in lowercase.
