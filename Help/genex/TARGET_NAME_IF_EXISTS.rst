TARGET_NAME_IF_EXISTS
---------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_NAME_IF_EXISTS:tgt>

.. versionadded:: 3.12

The target name ``tgt`` if the target exists, an empty string otherwise.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
