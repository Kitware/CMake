<LANG>_LINKER_LAUNCHER
----------------------

.. versionadded:: 3.21

This property is implemented only when ``<LANG>`` is one of:

* ``C``

* ``CXX``

* ``CUDA``

  .. versionadded:: 4.5

* ``OBJC``

* ``OBJCXX``

* ``Fortran``

  .. versionadded:: 4.1

* ``HIP``

  .. versionadded:: 4.5

* ``SYCL``

  .. versionadded:: 4.5

  .. note::
    Experimental. Gated by ``CMAKE_EXPERIMENTAL_SYCL``.

Specify a :ref:`semicolon-separated list <CMake Language Lists>` containing a
command line for a linker launching tool. The :ref:`Makefile Generators` and the
:generator:`Ninja` generator will run this tool and pass the linker and its
arguments to the tool. This is useful for tools such as static analyzers.

This property is initialized by the value of the
:variable:`CMAKE_<LANG>_LINKER_LAUNCHER` variable if it is set when a target is
created.

.. versionadded:: 3.27

  The property value may use
  :manual:`generator expressions <cmake-generator-expressions(7)>`.

The launcher is used only for linking executables, shared libraries, and
module libraries.  It is not used when creating static libraries.

.. versionadded:: 4.5

  Any wrappers specified by :prop_tgt:`LINK_WRAPPERS` and
  :prop_tgt:`INTERFACE_LINK_WRAPPERS` are placed before the launcher command
  line. See :prop_tgt:`LINK_WRAPPERS` for details.
