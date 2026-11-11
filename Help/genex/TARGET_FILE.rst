TARGET_FILE
-----------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_FILE:tgt>

Full path to the ``tgt`` binary file.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on, unless the expression is being used in
:command:`add_custom_command` or :command:`add_custom_target`.
