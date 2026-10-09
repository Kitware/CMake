SYCL_SEPARABLE_COMPILATION
--------------------------

.. versionadded:: 4.5

.. note::
   Experimental. Gated by ``CMAKE_EXPERIMENTAL_SYCL``.

SYCL only: Enables separate compilation of device code

If set this will enable separable compilation for all SYCL files for
the given target.

For instance:

.. code-block:: cmake

  set_property(TARGET myexe PROPERTY SYCL_SEPARABLE_COMPILATION ON)

This property is initialized by the value of the
:variable:`CMAKE_SYCL_SEPARABLE_COMPILATION` variable if it is set when a
target is created.
