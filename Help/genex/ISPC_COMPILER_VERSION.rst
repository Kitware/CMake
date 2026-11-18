ISPC_COMPILER_VERSION
---------------------

.. versionadded:: 3.19

.. |genex-family| replace:: :ref:`genex-family-compiler-version`

.. include:: ../manual/include/GENEX_FAMILY.rst

Synopsis
^^^^^^^^

.. parsed-literal::

  `$<ISPC_COMPILER_VERSION> <Without Arguments_>`_
  `$<ISPC_COMPILER_VERSION:version> <With Arguments_>`_

Without Arguments
^^^^^^^^^^^^^^^^^

.. code-block:: cmake

  $<ISPC_COMPILER_VERSION>

The version of the ISPC compiler used.

With Arguments
^^^^^^^^^^^^^^

.. code-block:: cmake

  $<ISPC_COMPILER_VERSION:version>

``1`` if the version of the ISPC compiler matches ``version``, otherwise ``0``.
