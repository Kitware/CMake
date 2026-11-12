AND
---

.. |genex-family| replace:: :ref:`genex-family-boolean-generator-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<AND:conditions>

where ``conditions`` is a comma-separated list of boolean expressions, all of
which must evaluate to either ``1`` or ``0``.  The whole expression evaluates to
``1`` if all conditions are ``1``.  If any condition is ``0``, the whole
expression evaluates to ``0``.
