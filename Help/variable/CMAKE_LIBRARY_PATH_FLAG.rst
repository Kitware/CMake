CMAKE_LIBRARY_PATH_FLAG
-----------------------

The flag to be used to add a library search path to a compiler.

The flag will be used to specify a library directory to the compiler.
On most compilers this is ``-L``.

If the per-language :variable:`CMAKE_<LANG>_LIBRARY_PATH_FLAG` variable
is defined, it takes precedence over :variable:`!CMAKE_LIBRARY_PATH_FLAG`.
