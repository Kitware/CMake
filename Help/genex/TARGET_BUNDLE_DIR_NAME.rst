TARGET_BUNDLE_DIR_NAME
----------------------

.. versionadded:: 3.24

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

.. code-block:: cmake

  $<TARGET_BUNDLE_DIR_NAME:tgt>

Name of the bundle directory (``my.app``, ``my.framework``, or ``my.bundle``),
where ``tgt`` is the name of a target.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on (see policy :policy:`CMP0112`).
