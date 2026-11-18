HIP_COMPILER_FRONTEND_VARIANT
-----------------------------

.. versionadded:: 3.30

.. |genex-family| replace::
   :ref:`genex-family-compiler-language-id-and-frontend-variant`

.. include:: ../manual/include/GENEX_FAMILY.rst

Synopsis
^^^^^^^^

.. parsed-literal::

  `$<HIP_COMPILER_FRONTEND_VARIANT> <Without Arguments_>`_
  `$<HIP_COMPILER_FRONTEND_VARIANT:compiler_ids> <With Arguments_>`_

Without Arguments
^^^^^^^^^^^^^^^^^

.. code-block:: cmake

  $<HIP_COMPILER_FRONTEND_VARIANT>

CMake's compiler id of the HIP compiler used.

With Arguments
^^^^^^^^^^^^^^

.. code-block:: cmake

  $<HIP_COMPILER_FRONTEND_VARIANT:compiler_ids>

where ``compiler_ids`` is a comma-separated list. ``1`` if CMake's compiler
frontend variant of the HIP compiler matches any one of the entries in
``compiler_ids``, otherwise ``0``.
