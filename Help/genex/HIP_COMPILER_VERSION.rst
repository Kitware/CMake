HIP_COMPILER_VERSION
--------------------

.. versionadded:: 3.21

.. |genex-family| replace:: :ref:`genex-family-compiler-version`

.. include:: ../manual/include/GENEX_FAMILY.rst

Synopsis
^^^^^^^^

.. parsed-literal::

  `$<HIP_COMPILER_VERSION> <Without Arguments_>`_
  `$<HIP_COMPILER_VERSION:version> <With Arguments_>`_

Without Arguments
^^^^^^^^^^^^^^^^^

.. code-block:: cmake

  $<HIP_COMPILER_VERSION>

The version of the HIP compiler used.

With Arguments
^^^^^^^^^^^^^^

.. code-block:: cmake

  $<HIP_COMPILER_VERSION:version>

``1`` if the version of the HIP compiler matches ``version``, otherwise ``0``.
