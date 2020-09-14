CMAKE_XCODE_BUILD_SYSTEM
------------------------

Xcode build system selection.

The :generator:`Xcode` generator defines this variable to indicate which
variant of the Xcode build system will be used.  The value is the
version of Xcode in which the corresponding build system first became
mature enough for use by CMake.  The possible values are:

``1``
  The original Xcode build system.
  This is the default.

The ``CMAKE_XCODE_BUILD_SYSTEM`` variable is informational and should not
be modified by project code.  See the :ref:`Xcode Build System Selection`
documentation section to select the Xcode build system.
