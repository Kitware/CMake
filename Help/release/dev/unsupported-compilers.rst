unsupported-compilers
---------------------

* The implementation of CMake relies on some C++ compiler features which are
  not supported by some older compilers.  As a result, those old compilers
  can no longer be used to build CMake itself.  CMake continues to be able to
  generate Makefiles and project files for users of those old compilers
  however.  The compilers known to no longer be capable of building CMake
  are:

  * MSVC 6 and 7.0 - superceded by VisualStudio 7.1 and newer compilers.
  * GCC 2.95 - superceded by GCC 3 and newer compilers.
  * Borland compilers - superceded by other Windows compilers.
  * Compaq compilers - superceded by other compilers.
  * Comeau compilers - superceded by other compilers.
  * SGI compilers - IRIX was dropped as a host platform.

  When building using SolarisStudio 12, the default ``libCStd`` standard
  library is not sufficient to build CMake.  The SolarisStudio distribution
  supports compiler options to use ``STLPort4`` or ``libstdc++``.  An
  appropriate option to select the standard library is now added
  automatically when building CMake with SolarisStudio compilers.
