C_COMPILER_VERSION
------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<C_COMPILER_VERSION>

The version of the C compiler used.

Syntax
^^^^^^

.. code-block:: cmake

  $<C_COMPILER_VERSION:version>

``1`` if the version of the C compiler matches ``version``, otherwise ``0``.
