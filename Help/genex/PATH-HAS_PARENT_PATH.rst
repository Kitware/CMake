PATH:HAS_PARENT_PATH
--------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-queries`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:HAS_PARENT_PATH,path>

The following operations return ``1`` if the particular path component is
present, ``0`` otherwise. See :ref:`Path Structure And Terminology` for the
meaning of each path component.

Note that the root directory is also considered to have a parent, which will
be itself.  The result is true except if the path consists of just a
:ref:`filename <FILENAME_DEF>`.
