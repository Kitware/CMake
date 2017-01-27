genex-if
--------

* A new logical generator expression for immediate-if was added:
  ``$<IF:cond,true-value,false-value>``. It takes three arguments: One
  condition, a true-value, and a false-value. Resolves to the true-value if the
  condition is ``1``, and resolves to the false-value if the condition is ``0``.
