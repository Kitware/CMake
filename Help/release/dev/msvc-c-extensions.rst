msvc-c-extensions
-----------------

* The :prop_tgt:`C_EXTENSIONS` target property is now honored by the
  ``MSVC`` compiler from Visual Studio 2022 version 17.2 or above and by
  ``clang-cl`` 19.1 or above.  ``OFF`` adds ``/Zc:__STDC__`` for the C11
  and later standards, which disables the Microsoft and UCRT extensions.
  See policy :policy:`CMP0233`.
