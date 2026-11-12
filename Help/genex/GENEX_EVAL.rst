GENEX_EVAL
----------

.. |genex-family| replace::
   :ref:`genex-family-multi-level-expression-evaluation`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<GENEX_EVAL:expr>

.. versionadded:: 3.12

Content of ``expr`` evaluated as a generator expression in the current context.
This enables consumption of generator expressions whose evaluation results
itself in generator expressions.
