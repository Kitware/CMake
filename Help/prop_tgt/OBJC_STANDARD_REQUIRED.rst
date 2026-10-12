OBJC_STANDARD_REQUIRED
----------------------

.. versionadded:: 3.16

Boolean describing whether the value of :prop_tgt:`OBJC_STANDARD` is a requirement.

If this property is set to ``ON``, then the value of the
:prop_tgt:`OBJC_STANDARD` target property is treated as a requirement.  If this
property is ``OFF`` or unset, the :prop_tgt:`OBJC_STANDARD` target property is
treated as optional and may "decay" to a previous standard if the requested is
not available.

If the property is not set, and the project has set the :prop_tgt:`C_STANDARD_REQUIRED`,
the value of :prop_tgt:`C_STANDARD_REQUIRED` is set for ``OBJC_STANDARD_REQUIRED``.

.. include:: ../include/COMPILE_FEATURES_NOTE.rst

This property is initialized by the value of
the :variable:`CMAKE_OBJC_STANDARD_REQUIRED` variable if it is set when a
target is created.
