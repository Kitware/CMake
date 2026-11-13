TARGET_BUNDLE_DIR
-----------------

.. |genex-family| replace:: :ref:`genex-family-target-artifacts`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_BUNDLE_DIR:tgt>

.. versionadded:: 3.9

Full path to the bundle directory (``/path/to/my.app``,
``/path/to/my.framework``, or ``/path/to/my.bundle``), where ``tgt`` is the name
of a target.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on (see policy :policy:`CMP0112`).
