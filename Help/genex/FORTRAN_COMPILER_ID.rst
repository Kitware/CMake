Fortran_COMPILER_ID
-------------------

.. |genex-family| replace::
   :ref:`genex-family-compiler-language-id-and-frontend-variant`

.. include:: ../manual/include/GENEX_FAMILY.rst

Synopsis
^^^^^^^^

.. parsed-literal::

  `$<Fortran_COMPILER_ID> <Without Arguments_>`_
  `$<Fortran_COMPILER_ID:compiler_ids> <With Arguments_>`_

Without Arguments
^^^^^^^^^^^^^^^^^

.. code-block:: cmake

  $<Fortran_COMPILER_ID>

CMake's compiler id of the Fortran compiler used.

With Arguments
^^^^^^^^^^^^^^

.. code-block:: cmake

  $<Fortran_COMPILER_ID:compiler_ids>

where ``compiler_ids`` is a comma-separated list. ``1`` if CMake's compiler id
of the Fortran compiler matches any one of the entries in ``compiler_ids``,
otherwise ``0``.

.. versionchanged:: 3.15
  Multiple ``compiler_ids`` can be specified.
  CMake 3.14 and earlier only accepted a single compiler ID.
