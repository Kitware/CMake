SYCL_CXX_STANDARD_REQUIRED
--------------------------

.. versionadded:: 4.5

.. note::
   Experimental. Gated by ``CMAKE_EXPERIMENTAL_SYCL``.

Boolean describing whether the value of :prop_tgt:`SYCL_CXX_STANDARD` is a
requirement.

If this property is set to ``ON``, then the value of the
:prop_tgt:`SYCL_CXX_STANDARD` target property is treated as a requirement.  If this
property is ``OFF`` or unset, the :prop_tgt:`SYCL_CXX_STANDARD` target property is
treated as optional and may "decay" to a previous standard if the requested is
not available.

See the :manual:`cmake-compile-features(7)` manual for information on
compile features and a list of supported compilers.

This property is initialized by the value of
the :variable:`CMAKE_SYCL_CXX_STANDARD_REQUIRED` variable if it is set when a
target is created.
