SYCL_DEVICE_TARGETS
-------------------

.. versionadded:: 4.5

.. note::
   Experimental. Gated by ``CMAKE_EXPERIMENTAL_SYCL``.

List of device targets to generate code for.  Names are interpreted by the
SYCL compiler:

* AdaptiveCpp accepts compilation flows with optional architecture lists,
  such as ``generic``, ``omp.accelerated``,
  ``cuda.explicit-multipass:sm_80,sm_86``, and
  ``hip.explicit-multipass:gfx90a``.
* IntelLLVM accepts SYCL target triples or architecture aliases, such as
  ``spir64`` and ``intel_gpu_pvc``.
* Clang accepts offload target triples, such as ``spirv64-unknown-unknown``.

The compiler installation must support the requested targets.  These values
are not interchangeable across compilers.

This property is initialized by :variable:`CMAKE_SYCL_DEVICE_TARGETS` when a
target is created.  If unset or empty, the compiler's configure-time device
target selection is used.  A non-empty false value (for example ``OFF``)
suppresses CMake-generated device target options.

For example:

.. code-block:: cmake

   if(CMAKE_SYCL_COMPILER_ID STREQUAL "AdaptiveCpp")
     set_property(TARGET app PROPERTY SYCL_DEVICE_TARGETS
       cuda.explicit-multipass:sm_80)
   elseif(CMAKE_SYCL_COMPILER_ID STREQUAL "IntelLLVM")
     set_property(TARGET app PROPERTY SYCL_DEVICE_TARGETS intel_gpu_pvc)
   endif()

Selections are applied to SYCL compilation and linking, including standalone
device linking when the host link uses another language. Set compatible
selections on targets that compile device code and on the target that links
it; this property is not a transitive usage requirement.
