HIP_COMPILER_ID
---------------

Syntax
^^^^^^

.. code-block:: cmake

  $<HIP_COMPILER_ID>

.. versionadded:: 3.21

CMake's compiler id of the HIP compiler used.

Syntax
^^^^^^

.. code-block:: cmake

  $<HIP_COMPILER_ID:compiler_ids>

.. versionadded:: 3.21

where ``compiler_ids`` is a comma-separated list. ``1`` if CMake's compiler id
of the HIP compiler matches any one of the entries in ``compiler_ids``,
otherwise ``0``.
