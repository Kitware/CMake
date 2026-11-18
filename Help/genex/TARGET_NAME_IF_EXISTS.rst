TARGET_NAME_IF_EXISTS
---------------------

.. versionadded:: 3.12

.. |genex-family| replace:: :ref:`genex-family-target-meta-data`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<TARGET_NAME_IF_EXISTS:tgt>

The target name ``tgt`` if the target exists, an empty string otherwise.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on.
