Fortran_COMPILER_VERSION
------------------------

.. |genex-family| replace:: :ref:`genex-family-compiler-version`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<Fortran_COMPILER_VERSION>

The version of the Fortran compiler used.

Syntax
^^^^^^

.. code-block:: cmake

  $<Fortran_COMPILER_VERSION:version>

``1`` if the version of the Fortran compiler matches ``version``, otherwise
``0``.
