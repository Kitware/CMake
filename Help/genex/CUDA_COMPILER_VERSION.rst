CUDA_COMPILER_VERSION
---------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<CUDA_COMPILER_VERSION>

.. versionadded:: 3.15

The version of the CUDA compiler used.

Syntax
^^^^^^

.. code-block:: cmake

  $<CUDA_COMPILER_VERSION:version>

.. versionadded:: 3.15

``1`` if the version of the C++ compiler matches ``version``, otherwise ``0``.
