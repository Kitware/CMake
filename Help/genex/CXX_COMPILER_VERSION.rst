CXX_COMPILER_VERSION
--------------------

.. |genex-family| replace:: :ref:`genex-family-compiler-version`

.. include:: ../manual/include/GENEX_FAMILY.rst

Synopsis
^^^^^^^^

.. parsed-literal::

  `$<CXX_COMPILER_VERSION> <Without Arguments_>`_
  `$<CXX_COMPILER_VERSION:version> <With Arguments_>`_

Without Arguments
^^^^^^^^^^^^^^^^^

.. code-block:: cmake

  $<CXX_COMPILER_VERSION>

The version of the CXX compiler used.

With Arguments
^^^^^^^^^^^^^^

.. code-block:: cmake

  $<CXX_COMPILER_VERSION:version>

``1`` if the version of the C++ compiler matches ``version``, otherwise ``0``.
