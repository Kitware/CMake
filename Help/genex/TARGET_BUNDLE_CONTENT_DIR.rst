TARGET_BUNDLE_CONTENT_DIR
-------------------------

Syntax
^^^^^^

.. code-block:: cmake

  $<TARGET_BUNDLE_CONTENT_DIR:tgt>

.. versionadded:: 3.9

Full path to the bundle content directory where ``tgt`` is the name of a target.
For the macOS SDK it leads to ``/path/to/my.app/Contents``,
``/path/to/my.framework``, or ``/path/to/my.bundle/Contents``. For all other
SDKs (e.g. iOS) it leads to ``/path/to/my.app``, ``/path/to/my.framework``, or
``/path/to/my.bundle`` due to the flat bundle structure.

Note that ``tgt`` is not added as a dependency of the target this expression is
evaluated on (see policy :policy:`CMP0112`).
