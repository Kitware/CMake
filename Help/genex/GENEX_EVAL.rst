GENEX_EVAL
----------

.. versionadded:: 3.12

.. |genex-family| replace::
   :ref:`genex-family-multi-level-expression-evaluation`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<GENEX_EVAL:expr>

Content of ``expr`` evaluated as a generator expression in the current context.
This enables consumption of generator expressions whose evaluation results
itself in generator expressions.
