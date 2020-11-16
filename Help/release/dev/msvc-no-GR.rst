msvc-no-GR
----------

* With MSVC-like compilers the value of
  :variable:`CMAKE_CXX_FLAGS <CMAKE_<LANG>_FLAGS>` no longer contains
  the ``/GR`` flag for runtime type information by default.
  See policy :policy:`CMP0117`.
