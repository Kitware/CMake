TARGET_BUNDLE_DIR_NAME
----------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_BUNDLE_DIR_NAME:tgt>

.. versionadded:: 3.24

Name of the bundle directory (``my.app``, ``my.framework``, or ``my.bundle``),
where ``tgt`` is the name of a target.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on (see policy :policy:`CMP0112`).
