HIP_COMPILER_VERSION
--------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<HIP_COMPILER_VERSION>

.. versionadded:: 3.21

The version of the HIP compiler used.

Syntax
^^^^^^

.. code-block:: cmake

  $<HIP_COMPILER_VERSION:version>

.. versionadded:: 3.21

``1`` if the version of the HIP compiler matches ``version``, otherwise ``0``.
