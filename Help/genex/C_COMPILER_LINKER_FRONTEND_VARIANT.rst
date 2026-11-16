C_COMPILER_LINKER_FRONTEND_VARIANT
----------------------------------

.. versionadded:: 4.2

.. |genex-family| replace:: :ref:`genex-family-linker-id-and-frontend-variant`

.. include:: ../manual/include/GENEX_FAMILY.rst

Synopsis
^^^^^^^^

.. parsed-literal::

  `$<C_COMPILER_LINKER_FRONTEND_VARIANT> <Without Arguments_>`_
  `$<C_COMPILER_LINKER_FRONTEND_VARIANT:variant_ids> <With Arguments_>`_

Without Arguments
^^^^^^^^^^^^^^^^^

.. code-block:: cmake

  $<C_COMPILER_LINKER_FRONTEND_VARIANT>

CMake's linker frontend variant of the C linker used.

With Arguments
^^^^^^^^^^^^^^

.. code-block:: cmake

  $<C_COMPILER_LINKER_FRONTEND_VARIANT:variant_ids>

where ``variant_ids`` is a comma-separated list. ``1`` if CMake's linker
frontend variant of the C linker matches any one of the entries in
``variant_ids``, otherwise ``0``.
