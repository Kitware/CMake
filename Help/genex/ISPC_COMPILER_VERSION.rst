ISPC_COMPILER_VERSION
---------------------

.. |genex-family| replace:: :ref:`genex-family-compiler-version`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<ISPC_COMPILER_VERSION>

.. versionadded:: 3.19

The version of the ISPC compiler used.

Syntax
^^^^^^

.. code-block:: cmake

  $<ISPC_COMPILER_VERSION:version>

.. versionadded:: 3.19

``1`` if the version of the ISPC compiler matches ``version``, otherwise ``0``.
