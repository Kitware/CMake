OBJC_COMPILER_VERSION
---------------------

.. |genex-family| replace:: :ref:`genex-family-compiler-version`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<OBJC_COMPILER_VERSION>

.. versionadded:: 3.16

The version of the Objective-C compiler used.

Syntax
^^^^^^

.. code-block:: cmake

  $<OBJC_COMPILER_VERSION:version>

.. versionadded:: 3.16

``1`` if the version of the Objective-C compiler matches ``version``, otherwise
``0``.
