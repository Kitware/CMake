TARGET_SONAME_FILE
------------------

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_SONAME_FILE:tgt>

File with soname (``.so.3``) where ``tgt`` is the name of a target.
