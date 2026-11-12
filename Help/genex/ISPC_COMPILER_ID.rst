ISPC_COMPILER_ID
----------------

.. |genex-family| replace::
   :ref:`genex-family-compiler-language-id-and-frontend-variant`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<ISPC_COMPILER_ID>

.. versionadded:: 3.19

CMake's compiler id of the ISPC compiler used.

Syntax
^^^^^^

.. code-block:: cmake

  $<ISPC_COMPILER_ID:compiler_ids>

.. versionadded:: 3.19

where ``compiler_ids`` is a comma-separated list. ``1`` if CMake's compiler id
of the ISPC compiler matches any one of the entries in ``compiler_ids``,
otherwise ``0``.
