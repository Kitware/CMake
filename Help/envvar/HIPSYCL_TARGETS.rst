HIPSYCL_TARGETS
---------------

.. versionadded:: 4.5

.. note::
   Experimental. Gated by ``CMAKE_EXPERIMENTAL_SYCL``.

.. include:: include/ENV_VAR.rst

Legacy name for :envvar:`ACPP_TARGETS`.  CMake reads this variable when first
enabling SYCL with AdaptiveCpp only if neither
:variable:`CMAKE_SYCL_DEVICE_TARGETS` nor the :envvar:`ACPP_TARGETS`
environment variable is defined.  It uses the same target format and
configure-time default behavior as ``ACPP_TARGETS``.
