CXX_COMPILER_VERSION
--------------------

.. |genex-family| replace:: :ref:`genex-family-compiler-version`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<CXX_COMPILER_VERSION>

The version of the CXX compiler used.

Syntax
^^^^^^

.. code-block:: cmake

  $<CXX_COMPILER_VERSION:version>

``1`` if the version of the C++ compiler matches ``version``, otherwise ``0``.
