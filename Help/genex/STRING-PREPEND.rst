STRING:PREPEND
--------------

.. |genex-family| replace::
   :ref:`genex-family-string-transforming-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<STRING:PREPEND,string,input[,input]...>

.. versionadded:: 4.3

Prepend all the ``input`` arguments to the ``string``.
