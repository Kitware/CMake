PATH:HAS_PARENT_PATH
--------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-queries`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:HAS_PARENT_PATH,path>

Returns ``1`` if ``path`` has a parent path, ``0`` otherwise. The parent path
is the path with its filename removed.

Note that the root directory is also considered to have a parent, which will
be itself.  The result is true except if the path consists of just a
:ref:`filename <FILENAME_DEF>`.
