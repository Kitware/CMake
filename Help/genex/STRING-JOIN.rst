STRING:JOIN
-----------

.. |genex-family| replace::
   :ref:`genex-family-string-generating-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<STRING:JOIN,glue,input[,input]...>

.. versionadded:: 4.3

Join all the ``input`` arguments together using the ``glue`` string.
