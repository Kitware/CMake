define-windll
-------------

* For builds targeting the MSVC ABI, all generators now add the ``_WINDLL``
  preprocessor definition when compiling sources in shared libraries.
  See policy :policy:`CMP0203`.
