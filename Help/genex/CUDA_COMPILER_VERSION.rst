CUDA_COMPILER_VERSION
---------------------

.. versionadded:: 3.15

.. |genex-family| replace:: :ref:`genex-family-compiler-version`

.. include:: ../manual/include/GENEX_FAMILY.rst

Synopsis
^^^^^^^^

.. parsed-literal::

  `$<CUDA_COMPILER_VERSION> <Without Arguments_>`_
  `$<CUDA_COMPILER_VERSION:version> <With Arguments_>`_

Without Arguments
^^^^^^^^^^^^^^^^^

.. code-block:: cmake

  $<CUDA_COMPILER_VERSION>

The version of the CUDA compiler used.

With Arguments
^^^^^^^^^^^^^^

.. code-block:: cmake

  $<CUDA_COMPILER_VERSION:version>

``1`` if the version of the C++ compiler matches ``version``, otherwise ``0``.
