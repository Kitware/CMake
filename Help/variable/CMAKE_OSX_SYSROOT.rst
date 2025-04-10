CMAKE_OSX_SYSROOT
-----------------

Specify the location or name of the macOS platform SDK to be used.
CMake uses this value to compute the value of the ``-isysroot`` flag
or equivalent and to help the ``find_*`` commands locate files in
the SDK.

If not set explicitly, the value is initialized by the ``SDKROOT``
environment variable, if set.  Otherwise, the value defaults to empty,
so no explicit ``-isysroot`` flag is passed, and the compiler's default
sysroot is used.

.. versionchanged:: 4.0
  The default is now empty.  Previously a default was computed based on
  the :variable:`CMAKE_OSX_DEPLOYMENT_TARGET` or the host platform.

.. note::

  Xcode's compilers, when not invoked with ``-isysroot``, search for
  headers in ``/usr/local/include`` before system SDK paths, matching the
  convention on many platforms.  Users on macOS-x86_64 hosts with Homebrew
  installed in ``/usr/local`` should pass ``-DCMAKE_OSX_SYSROOT=macosx``,
  or ``export SDKROOT=macosx``, when not building with Homebrew tools.

.. include:: CMAKE_OSX_VARIABLE.txt
