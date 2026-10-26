CMAKE_SKIP_INSTALL_RPATH
------------------------

Do not include ``RPATHs`` in the install tree.

Normally CMake uses the build tree for the ``RPATH`` when building
executables etc on systems that use ``RPATH``.  When the software is
installed the executables etc are relinked by CMake to have the
install ``RPATH``.  If this variable is set to true then the software is
always installed without ``RPATH``, even if ``RPATH`` is enabled when
building.  This can be useful for example to allow running tests from
the build directory with ``RPATH`` enabled before the installation step.

.. versionchanged:: 4.6
  This variable is used to initialize the :prop_tgt:`SKIP_INSTALL_RPATH` target
  property when a target is created.  See policy :policy:`CMP0229`.

For more information on RPATH handling see
the :prop_tgt:`INSTALL_RPATH` and :prop_tgt:`BUILD_RPATH` target properties.

See also the :variable:`CMAKE_SKIP_BUILD_RPATH` variable.
To omit ``RPATH`` in both the build and install steps, use
:variable:`CMAKE_SKIP_RPATH` instead.
