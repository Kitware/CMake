intel-llvm-compilers
--------------------

* The Intel oneAPI NextGen LLVM compilers are now supported with
  compiler id ``IntelLLVM``:

  * The ``icx``/``icpx`` C/C++ compilers on Linux, and the ``icx``
    C/C++ compiler on Windows, are fully supported as of oneAPI 2021.1.

  * The ``ifx`` Fortran compiler on Linux is partially supported.
    As of oneAPI 2021.1, ``ifx`` does not define several identification
    macros, so CMake identifies it as the classic ``Intel`` compiler.
    This works in many cases because ``ifx`` accepts the same command line
    parameters as ``ifort``.

  * The ``ifx`` Fortran compiler on Windows is not yet supported.

  The Intel oneAPI Classic compilers (``icc``, ``icpc``, and ``ifort``)
  continue to be supported with compiler id ``Intel``.
