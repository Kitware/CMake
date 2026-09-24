CMAKE_SYCL_DEVICE_TARGETS
-------------------------

.. versionadded:: 4.5

.. note::
   Experimental. Gated by ``CMAKE_EXPERIMENTAL_SYCL``.

Default value of the :prop_tgt:`SYCL_DEVICE_TARGETS` target property.  See
that property for the supported compiler-specific values.

If set before first enabling SYCL, this variable also selects device targets for
compiler identification and testing.

For AdaptiveCpp, if this variable is not defined when SYCL is first enabled,
the default selection is taken from the :envvar:`ACPP_TARGETS` environment
variable, then :envvar:`HIPSYCL_TARGETS`, or the compiler's installation
default if neither environment variable is defined.

This variable is propagated into :command:`try_compile` projects.
