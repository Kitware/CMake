CUDA_COMPILER_LINKER_FRONTEND_VARIANT
-------------------------------------

.. |genex-family| replace:: :ref:`genex-family-linker-id-and-frontend-variant`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<CUDA_COMPILER_LINKER_FRONTEND_VARIANT>

.. versionadded:: 4.2

CMake's linker frontend variant of the CUDA linker used.

Syntax
^^^^^^

.. code-block:: cmake

  $<CUDA_COMPILER_LINKER_FRONTEND_VARIANT:variant_ids>

.. versionadded:: 4.2

where ``variant_ids`` is a comma-separated list. ``1`` if CMake's linker
frontend variant of the CUDA linker matches any one of the entries in
``variant_ids``, otherwise ``0``.
