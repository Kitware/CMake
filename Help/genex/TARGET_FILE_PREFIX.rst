TARGET_FILE_PREFIX
------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_FILE_PREFIX:tgt>

.. versionadded:: 3.15

Prefix of the ``tgt`` filename (such as ``lib``).

See also the :prop_tgt:`PREFIX` target property.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
