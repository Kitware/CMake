SYCL_COMPILER_LINKER_FRONTEND_VARIANT
-------------------------------------

.. |genex-family| replace:: :ref:`genex-family-linker-id-and-frontend-variant`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<SYCL_COMPILER_LINKER_FRONTEND_VARIANT>

.. versionadded:: 4.5

.. note::
  Experimental. Gated by ``CMAKE_EXPERIMENTAL_SYCL``.

CMake's linker frontend variant of the SYCL linker used.

Syntax
^^^^^^

.. code-block:: cmake

  $<SYCL_COMPILER_LINKER_FRONTEND_VARIANT:variant_ids>

.. versionadded:: 4.5

.. note::
  Experimental. Gated by ``CMAKE_EXPERIMENTAL_SYCL``.

where ``variant_ids`` is a comma-separated list. ``1`` if CMake's linker
frontend variant of the SYCL linker matches any one of the entries in
``variant_ids``, otherwise ``0``.
