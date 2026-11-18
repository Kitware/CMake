STRING:MAKE_C_IDENTIFIER
------------------------

.. versionadded:: 4.3

.. |genex-family| replace::
   :ref:`genex-family-string-transforming-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<STRING:MAKE_C_IDENTIFIER,string>

Convert each non-alphanumeric character in the ``string`` to an underscore. If
the first character of the ``string`` is a digit, an underscore will also be
prepended.
