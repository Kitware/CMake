<LANG>_COMPILER_LAUNCHER
------------------------

.. versionadded:: 3.4

This property is implemented only when ``<LANG>`` is ``C``, ``CXX``,
``Fortran``, ``HIP``, ``ISPC``, ``OBJC``, ``OBJCXX``, ``CUDA``, ``SYCL``,
``Rust``, or ``Swift``.

.. versionchanged:: 4.5
  Added support for ``SYCL``.

  .. note::
    SYCL support is experimental and gated by ``CMAKE_EXPERIMENTAL_SYCL``.

.. versionchanged:: 4.6
  Added support for ``Rust``.

  .. note::
    Rust support is experimental and gated by ``CMAKE_EXPERIMENTAL_RUST``.

.. versionchanged:: 4.6
  Added support for ``Swift``.

  .. note::
    Setting the compiler launcher for ``Swift`` is only supported when
    :policy:`CMP0157` is set to use the ``NEW`` behavior. This property has no
    effect when the policy is set to the ``OLD`` behavior.

Specify a :ref:`semicolon-separated list <CMake Language Lists>` containing a command line
for a compiler launching tool. The :ref:`Makefile Generators` and the
:generator:`Ninja` generator will run this tool and pass the compiler and
its arguments to the tool. Some example tools are distcc and ccache.

This property is initialized by the value of
the :variable:`CMAKE_<LANG>_COMPILER_LAUNCHER` variable if it is set
when a target is created.

.. versionadded:: 3.25

  The property value may use
  :manual:`generator expressions <cmake-generator-expressions(7)>`.

.. versionadded:: 4.5

  Any wrappers specified by :prop_tgt:`COMPILE_WRAPPERS` and
  :prop_tgt:`INTERFACE_COMPILE_WRAPPERS` are placed before the launcher
  command line.  See :prop_tgt:`COMPILE_WRAPPERS` for details.
