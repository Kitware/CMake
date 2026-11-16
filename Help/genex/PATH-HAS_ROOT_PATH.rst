PATH:HAS_ROOT_PATH
------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-queries`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:HAS_ROOT_PATH,path>

The following operations return ``1`` if the particular path component is
present, ``0`` otherwise. See :ref:`Path Structure And Terminology` for the
meaning of each path component.

The result is true only if at least one of the root name or root directory
separator is present.
