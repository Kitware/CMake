Fortran_COMPILER_ID
-------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<Fortran_COMPILER_ID>

CMake's compiler id of the Fortran compiler used.

Syntax
^^^^^^

.. code-block:: cmake

  $<Fortran_COMPILER_ID:compiler_ids>

where ``compiler_ids`` is a comma-separated list. ``1`` if CMake's compiler id
of the Fortran compiler matches any one of the entries in ``compiler_ids``,
otherwise ``0``.

.. versionchanged:: 3.15
  Multiple ``compiler_ids`` can be specified.
  CMake 3.14 and earlier only accepted a single compiler ID.
