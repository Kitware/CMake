OBJC_COMPILER_VERSION
---------------------

.. versionadded:: 3.16

.. |genex-family| replace:: :ref:`genex-family-compiler-version`

.. include:: ../manual/include/GENEX_FAMILY.rst

Synopsis
^^^^^^^^

.. parsed-literal::

  `$<OBJC_COMPILER_VERSION> <Without Arguments_>`_
  `$<OBJC_COMPILER_VERSION:version> <With Arguments_>`_

Without Arguments
^^^^^^^^^^^^^^^^^

.. code-block:: cmake

  $<OBJC_COMPILER_VERSION>

The version of the Objective-C compiler used.

With Arguments
^^^^^^^^^^^^^^

.. code-block:: cmake

  $<OBJC_COMPILER_VERSION:version>

``1`` if the version of the Objective-C compiler matches ``version``, otherwise
``0``.
