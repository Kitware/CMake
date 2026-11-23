msvc-preprocessor
-----------------

* The :prop_tgt:`C_EXTENSIONS` and :prop_tgt:`CXX_EXTENSIONS` target
  properties are now honored by the ``MSVC`` compiler from Visual Studio 2019
  version 16.5 or above for selecting the preprocessor.  ``OFF`` adds the
  ``/Zc:preprocessor`` flag, which selects the standard-conforming
  preprocessor.  A target that uses both languages gets the flag for both if
  either one disables extensions.  See policy :policy:`CMP0234`.
