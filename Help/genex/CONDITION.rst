condition
---------

Syntax
^^^^^^

.. code-block:: cmake

  $<condition:true_string>

Evaluates to ``true_string`` if ``condition`` is ``1``, or an empty string if
``condition`` evaluates to ``0``.  Any other value for ``condition`` results in
an error.
