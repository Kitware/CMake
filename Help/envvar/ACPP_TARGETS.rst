ACPP_TARGETS
------------

.. versionadded:: 4.5

.. note::
   Experimental. Gated by ``CMAKE_EXPERIMENTAL_SYCL``.

.. include:: include/ENV_VAR.rst

Default device target selection for the AdaptiveCpp SYCL compiler.  CMake
reads this variable when first enabling SYCL if
:variable:`CMAKE_SYCL_DEVICE_TARGETS` is not defined.  The selection is used
for compiler identification and testing, and as the default for targets
whose :prop_tgt:`SYCL_DEVICE_TARGETS` property is unset or empty.  Subsequent
configurations retain the detected default selection.

The value is a semicolon-separated list of compilation flows, each optionally
followed by a colon and a comma-separated list of architectures.  See
:prop_tgt:`SYCL_DEVICE_TARGETS` for examples of AdaptiveCpp target names.

For example:

.. code-block:: console

   $ export ACPP_TARGETS='cuda.explicit-multipass:sm_80,sm_86;hip.explicit-multipass:gfx90a'

If this environment variable is not defined, CMake also checks
:envvar:`HIPSYCL_TARGETS` before using the compiler's installation default.
