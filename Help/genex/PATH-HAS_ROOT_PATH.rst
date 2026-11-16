PATH:HAS_ROOT_PATH
------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-queries`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:HAS_ROOT_PATH,path>

Returns ``1`` if ``path`` has a root path, ``0`` otherwise. The root path is
the root name followed by the root directory separator; either component may
be empty.

The result is true only if at least one of the root name or root directory
separator is present.
