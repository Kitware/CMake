SYCL_COMPILER_LINKER_ID
-----------------------

.. versionadded:: 4.5

.. note::
  Experimental. Gated by ``CMAKE_EXPERIMENTAL_SYCL``.

.. |genex-family| replace:: :ref:`genex-family-linker-id-and-frontend-variant`

.. include:: ../manual/include/GENEX_FAMILY.rst

Synopsis
^^^^^^^^

.. parsed-literal::

  `$<SYCL_COMPILER_LINKER_ID> <Without Arguments_>`_
  `$<SYCL_COMPILER_LINKER_ID:linker_ids> <With Arguments_>`_

Without Arguments
^^^^^^^^^^^^^^^^^

.. code-block:: cmake

  $<SYCL_COMPILER_LINKER_ID>

CMake's linker id of the SYCL linker used.

With Arguments
^^^^^^^^^^^^^^

.. code-block:: cmake

  $<SYCL_COMPILER_LINKER_ID:linker_ids>

where ``linker_ids`` is a comma-separated list. ``1`` if CMake's linker id of
the SYCL linker matches any one of the entries in ``linker_ids``, otherwise
``0``.
