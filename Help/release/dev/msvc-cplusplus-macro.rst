msvc-cplusplus-macro
--------------------

* With the ``MSVC`` compiler from Visual Studio 2017 version 15.7 or above,
  the ``/Zc:__cplusplus`` flag is now passed when compiling C++ sources so
  that the ``__cplusplus`` preprocessor macro reports the selected C++
  standard.  See policy :policy:`CMP0232`.
