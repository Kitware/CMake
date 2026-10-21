FindOpenMP-clang-libomp
-----------------------

* The :module:`FindOpenMP` module, with Clang targeting the MSVC ABI, now
  prefers the OpenMP runtime shipped with the compiler, e.g. ``libomp.dll``,
  over the one shipped with MSVC.  Set ``OpenMP_libomp_LIBRARY`` to select
  a different library.
