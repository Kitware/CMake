C_COMPILER_LINKER_ID
--------------------

.. versionadded:: 4.2

.. |genex-family| replace:: :ref:`genex-family-linker-id-and-frontend-variant`

.. include:: ../manual/include/GENEX_FAMILY.rst

Synopsis
^^^^^^^^

.. parsed-literal::

  `$<C_COMPILER_LINKER_ID> <Without Arguments_>`_
  `$<C_COMPILER_LINKER_ID:linker_ids> <With Arguments_>`_

Without Arguments
^^^^^^^^^^^^^^^^^

.. code-block:: cmake

  $<C_COMPILER_LINKER_ID>

CMake's linker id of the C linker used.

With Arguments
^^^^^^^^^^^^^^

.. code-block:: cmake

  $<C_COMPILER_LINKER_ID:linker_ids>

where ``linker_ids`` is a comma-separated list. ``1`` if CMake's linker id of
the C linker matches any one of the entries in ``linker_ids``, otherwise ``0``.
