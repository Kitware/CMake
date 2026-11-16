PATH:HAS_ROOT_DIRECTORY
-----------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-queries`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:HAS_ROOT_DIRECTORY,path>

Returns ``1`` if ``path`` has a root directory, ``0`` otherwise. A root
directory is the separator at the beginning of a path that indicates an
absolute path.
