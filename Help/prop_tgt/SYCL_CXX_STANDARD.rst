SYCL_CXX_STANDARD
-----------------

.. versionadded:: 4.5

.. note::
   Experimental. Gated by ``CMAKE_EXPERIMENTAL_SYCL``.

The C++ standard whose features are requested for SYCL compilations of
this target.

This property specifies the C++ standard whose features are requested for
SYCL compilations of this target.  For some compilers, this results in adding
a flag such as ``-std=gnu++20`` to the compile line.  It selects the C++
language version, not the SYCL specification version.

Supported values are:

``98``
  C++98

``11``
  C++11

``14``
  C++14

``17``
  C++17

``20``
  C++20

``23``
  C++23

``26``
  C++26

If the value requested does not result in a compile flag being added for
the compiler in use, a previous standard flag will be added instead.  This
means that using:

.. code-block:: cmake

  set_property(TARGET tgt PROPERTY SYCL_CXX_STANDARD 11)

with a compiler which does not support ``-std=gnu++11`` or an equivalent
flag will not result in an error or warning, but will instead add the
``-std=gnu++98`` flag if supported.  This "decay" behavior may be controlled
with the :prop_tgt:`SYCL_CXX_STANDARD_REQUIRED` target property.
Additionally, the :prop_tgt:`SYCL_EXTENSIONS` target property may be used to
control whether compiler-specific extensions are enabled on a per-target basis.

See the :manual:`cmake-compile-features(7)` manual for information on
compile features and a list of supported compilers.

This property is initialized by the value of
the :variable:`CMAKE_SYCL_CXX_STANDARD` variable if it is set when a target
is created.
