ISPC_COMPILER_FRONTEND_VARIANT
------------------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<ISPC_COMPILER_FRONTEND_VARIANT>

.. versionadded:: 3.30

CMake's compiler id of the ISPC compiler used.

Syntax
^^^^^^

.. code-block:: cmake

  $<ISPC_COMPILER_FRONTEND_VARIANT:compiler_ids>

.. versionadded:: 3.30

where ``compiler_ids`` is a comma-separated list. ``1`` if CMake's compiler
frontend variant of the ISPC compiler matches any one of the entries in
``compiler_ids``, otherwise ``0``.
