IF
--
Syntax
^^^^^^

.. code-block:: cmake

  $<IF:condition,true_string,false_string>

.. versionadded:: 3.8

Evaluates to ``true_string`` if ``condition`` is ``1``, or ``false_string`` if
``condition`` is ``0``.  Any other value for ``condition`` results in an error.

.. versionadded:: 3.28

  This generator expression short-circuits such that generator expressions in
  ``false_string`` will not evaluate when ``condition`` is ``1``, and generator
  expressions in ``true_string`` will not evaluate when condition is ``0``.
