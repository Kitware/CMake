PATH:HAS_RELATIVE_PART
----------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-genex-path-queries`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<PATH:HAS_RELATIVE_PART,path>

The following operations return ``1`` if the particular path component is
present, ``0`` otherwise. See :ref:`Path Structure And Terminology` for the
meaning of each path component.
