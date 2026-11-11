CUDA_COMPILER_ID
----------------

Syntax
^^^^^^

.. code-block:: cmake

  $<CUDA_COMPILER_ID>

.. versionadded:: 3.15

CMake's compiler id of the CUDA compiler used.

Syntax
^^^^^^

.. code-block:: cmake

  $<CUDA_COMPILER_ID:compiler_ids>

.. versionadded:: 3.15

where ``compiler_ids`` is a comma-separated list. ``1`` if CMake's compiler id
of the CUDA compiler matches any one of the entries in ``compiler_ids``,
otherwise ``0``.
