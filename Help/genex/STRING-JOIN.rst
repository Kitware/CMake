STRING:JOIN
-----------

Syntax
^^^^^^

.. code-block:: cmake

  $<STRING:JOIN,glue,input[,input]...>

.. versionadded:: 4.3

Join all the ``input`` arguments together using the ``glue`` string.
