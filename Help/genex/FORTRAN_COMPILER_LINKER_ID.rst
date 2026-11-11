Fortran_COMPILER_LINKER_ID
--------------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<Fortran_COMPILER_LINKER_ID>

.. versionadded:: 4.2

CMake's linker id of the Fortran linker used.

Syntax
^^^^^^

.. code-block:: cmake

  $<Fortran_COMPILER_LINKER_ID:linker_ids>

.. versionadded:: 4.2

where ``linker_ids`` is a comma-separated list. ``1`` if CMake's linker id of
the Fortran linker matches any one of the entries in ``linker_ids``, otherwise
``0``.
