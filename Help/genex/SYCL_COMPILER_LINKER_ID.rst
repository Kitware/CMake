SYCL_COMPILER_LINKER_ID
-----------------------

.. |genex-family| replace:: :ref:`genex-family-linker-id-and-frontend-variant`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<SYCL_COMPILER_LINKER_ID>

.. versionadded:: 4.5

.. note::
  Experimental. Gated by ``CMAKE_EXPERIMENTAL_SYCL``.

CMake's linker id of the SYCL linker used.

Syntax
^^^^^^

.. code-block:: cmake

  $<SYCL_COMPILER_LINKER_ID:linker_ids>

.. versionadded:: 4.5

.. note::
  Experimental. Gated by ``CMAKE_EXPERIMENTAL_SYCL``.

where ``linker_ids`` is a comma-separated list. ``1`` if CMake's linker id of
the SYCL linker matches any one of the entries in ``linker_ids``, otherwise
``0``.
