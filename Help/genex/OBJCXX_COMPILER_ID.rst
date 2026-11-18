OBJCXX_COMPILER_ID
------------------

.. versionadded:: 3.16

.. |genex-family| replace::
   :ref:`genex-family-compiler-language-id-and-frontend-variant`

.. include:: ../manual/include/GENEX_FAMILY.rst

Synopsis
^^^^^^^^

.. parsed-literal::

  `$<OBJCXX_COMPILER_ID> <Without Arguments_>`_
  `$<OBJCXX_COMPILER_ID:compiler_ids> <With Arguments_>`_

Without Arguments
^^^^^^^^^^^^^^^^^

.. code-block:: cmake

  $<OBJCXX_COMPILER_ID>

CMake's compiler id of the Objective-C++ compiler used.

With Arguments
^^^^^^^^^^^^^^

.. code-block:: cmake

  $<OBJCXX_COMPILER_ID:compiler_ids>

where ``compiler_ids`` is a comma-separated list. ``1`` if CMake's compiler id
of the Objective-C++ compiler matches any one of the entries in
``compiler_ids``, otherwise ``0``.
