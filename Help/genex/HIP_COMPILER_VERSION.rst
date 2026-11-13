HIP_COMPILER_VERSION
--------------------

.. |genex-family| replace:: :ref:`genex-family-compiler-version`

.. include:: ../manual/include/GENEX_FAMILY.rst

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
