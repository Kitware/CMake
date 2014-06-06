OpenWatcom
----------

* Support for the Open Watcom compiler has been overhauled.
  The :variable:`CMAKE_<LANG>_COMPILER_ID` is now ``OpenWatcom``,
  and the :variable:`CMAKE_<LANG>_COMPILER_VERSION` now uses
  the Open Watcom external version numbering.  The external
  version numbers are lower than the internal version number
  by 11.
