Fortran_COMPILER_VERSION
------------------------

.. |genex-family| replace:: :ref:`genex-family-compiler-version`

.. include:: ../manual/include/GENEX_FAMILY.rst

Synopsis
^^^^^^^^

.. parsed-literal::

  `$<Fortran_COMPILER_VERSION> <Without Arguments_>`_
  `$<Fortran_COMPILER_VERSION:version> <With Arguments_>`_

Without Arguments
^^^^^^^^^^^^^^^^^

.. code-block:: cmake

  $<Fortran_COMPILER_VERSION>

The version of the Fortran compiler used.

With Arguments
^^^^^^^^^^^^^^

.. code-block:: cmake

  $<Fortran_COMPILER_VERSION:version>

``1`` if the version of the Fortran compiler matches ``version``, otherwise
``0``.
