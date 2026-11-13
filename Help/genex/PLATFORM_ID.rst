PLATFORM_ID
-----------

.. |genex-family| replace::
   :ref:`genex-family-toolchain-and-language-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<PLATFORM_ID>

The current system's CMake platform id. See also the
:variable:`CMAKE_SYSTEM_NAME` variable.

Syntax
^^^^^^

.. code-block:: cmake

  $<PLATFORM_ID:platform_ids>

``1`` if CMake's platform id matches any one of the entries in comma-separated
list ``platform_ids``, otherwise ``0``. See also the
:variable:`CMAKE_SYSTEM_NAME` variable.
