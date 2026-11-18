STRING:PREPEND
--------------

.. versionadded:: 4.3

.. |genex-family| replace::
   :ref:`genex-family-string-transforming-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<STRING:PREPEND,string,input[,input]...>

Prepend all the ``input`` arguments to the ``string``.
