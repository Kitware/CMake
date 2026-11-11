NOT
---

Syntax
^^^^^^

.. code-block:: cmake

  $<NOT:condition>

``condition`` must be ``0`` or ``1``.  The result of the expression is ``0`` if
``condition`` is ``1``, else ``1``.
