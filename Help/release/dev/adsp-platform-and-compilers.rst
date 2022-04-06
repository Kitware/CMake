adsp-platform-and-compilers
---------------------------

* The ADSP compiler (SHARC and Blackfin) now supports
  both CCES and VDSP++ installations,
  with required configuration now done in the compiler module itself
  rather than the Generic-ADSP platform module.

* A dedicated ``ADSP`` platform has been added
  to replace the existing ``Generic-ADSP`` implementation.
  This features automatic detection of the latest CCES/VDSP++ install
  and compiler selection (``cc21k`` vs. ``ccblkfn``)
  based off of the :variable:`CMAKE_SYSTEM_PROCESSOR` variable.
