OR
--
Syntax
^^^^^^

.. code-block:: cmake

  $<OR:conditions>

where ``conditions`` is a comma-separated list of boolean expressions. all of
which must evaluate to either ``1`` or ``0``.  The whole expression evaluates to
``1`` if at least one of the ``conditions`` is ``1``.  If all ``conditions``
evaluate to ``0``, the whole expression evaluates to ``0``.
