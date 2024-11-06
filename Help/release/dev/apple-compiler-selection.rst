apple-compiler-selection
------------------------

* Builds targeting macOS no longer choose any SDK or pass an ``-isysroot``
  flag to the compiler by default.  Instead, compilers are expected to
  choose a default macOS SDK on their own.  In order to use a compiler that
  does not do this, users must now specify ``-DCMAKE_OSX_SYSROOT=macosx``
  when configuring their build.

* On macOS with :ref:`Ninja Generators` and :ref:`Makefile Generators`, when
  a compiler is found in ``/usr/bin``, it is now used as-is and is no longer
  mapped to the corresponding compiler inside Xcode.  The mapping was
  introduced by CMake 3.2 to allow build trees to continue to work with their
  original compiler even when ``xcode-select`` switches to a different
  Xcode installation.  However, the compilers inside Xcode cannot be used
  without explicit ``-isysroot`` flags and are therefore not suitable for
  passing to arbitrary third-party build systems.  Furthermore, the mapping
  behavior can override user-specified compiler paths.  Therefore, this
  behavior has been reverted.
