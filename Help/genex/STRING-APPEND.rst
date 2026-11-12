STRING:APPEND
-------------

.. |genex-family| replace::
   :ref:`genex-family-string-transforming-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<STRING:APPEND,string,input[,input]...>

.. versionadded:: 4.3

Append all the ``input`` arguments to the ``string``.
