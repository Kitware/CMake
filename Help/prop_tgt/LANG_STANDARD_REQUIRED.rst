<LANG>_STANDARD_REQUIRED
------------------------

The variations are:

* :prop_tgt:`C_STANDARD_REQUIRED`
* :prop_tgt:`CXX_STANDARD_REQUIRED`
* :prop_tgt:`CUDA_STANDARD_REQUIRED`
* :prop_tgt:`HIP_STANDARD_REQUIRED`
* :prop_tgt:`OBJC_STANDARD_REQUIRED`
* :prop_tgt:`OBJCXX_STANDARD_REQUIRED`

These properties specify whether the value of :prop_tgt:`<LANG>_STANDARD` is a
requirement. When ``OFF`` or unset, the :prop_tgt:`<LANG>_STANDARD` target
property is treated as optional and may "decay" to a previous standard if the
requested is not available.

These properties are initialized by the value of the
:variable:`CMAKE_<LANG>_STANDARD_REQUIRED` variable if it is set when a target
is created.

For supported CMake versions see the respective pages.
To control language standard versions see :prop_tgt:`<LANG>_STANDARD`.

See the :manual:`cmake-compile-features(7)` manual for information on
compile features and a list of supported compilers.
