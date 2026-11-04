SKIP_INSTALL_RPATH
------------------

.. versionadded:: 4.6

Should rpaths be used for the install tree.

``SKIP_INSTALL_RPATH`` is a boolean property that when set to true then
the software is always installed without ``RPATH``, even if ``RPATH`` is
enabled when building.  This can be useful for example to allow running
tests from the build directory with ``RPATH`` enabled before the
installation step.

See also the :prop_tgt:`INSTALL_RPATH` target property.

This property is initialized by the value of the variable
:variable:`CMAKE_SKIP_INSTALL_RPATH` when policy :policy:`CMP0229` is
set to ``NEW``.
