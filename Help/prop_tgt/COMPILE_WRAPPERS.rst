COMPILE_WRAPPERS
----------------

.. versionadded:: 4.5

List of wrapper names to be prepended to the compile commands for a target's
sources.

This property holds a :ref:`semicolon-separated list <CMake Language Lists>`
of names, where each item is a ``<NAME>`` keyed to the
:prop_gbl:`CMAKE_COMPILE_WRAPPER_<NAME>` global property.

.. note::

  If a target has multiple wrappers, either via its ``COMPILE_WRAPPERS`` and/or
  the :prop_tgt:`INTERFACE_COMPILE_WRAPPERS` inherited from its dependents, the
  wrappers will be deduplicated and ordered lexicographically based on their
  name in the resulting compile command(s).

This property is supported only by :ref:`Makefile Generators` and
:ref:`Ninja Generators`, and is ignored by other generators. Further, it is
supported only when the target's language is one of: ``C``, ``CXX``, ``CUDA``,
``Fortran``, ``HIP``, ``ISPC``, ``OBJC``, ``OBJCXX``.

Contents of ``COMPILE_WRAPPERS`` may use "generator expressions" with the
syntax ``$<...>``.  See the :manual:`cmake-generator-expressions(7)` manual for
available expressions.  See the :manual:`cmake-buildsystem(7)` manual for more
on defining buildsystem properties.

Interaction With Launchers
^^^^^^^^^^^^^^^^^^^^^^^^^^

Wrappers are placed *outside* of any :prop_tgt:`<LANG>_COMPILER_LAUNCHER`.
The resulting compile command is:

.. code-block:: shell

  <wrappers...> <launcher...> <compiler> <arguments...>

A tool that expects to be given the compiler as its first argument, such as
``ccache`` or ``distcc``, should be used as a launcher rather than a wrapper.

When a static analysis tool is enabled for a source via
:prop_tgt:`<LANG>_CLANG_TIDY`, :prop_tgt:`<LANG>_CPPCHECK`,
:prop_tgt:`<LANG>_CPPLINT`, :prop_tgt:`<LANG>_INCLUDE_WHAT_YOU_USE`, or a
similar property, the wrappers and the launcher apply only to the compiler
invocation and *not* to the analysis tool.

The launcher used by :manual:`ctest(1)` (see :variable:`CTEST_USE_LAUNCHERS`)
remains outermost and runs the wrappers as part of the command it launches.

See Also
^^^^^^^^

Related properties:

* :prop_tgt:`INTERFACE_COMPILE_WRAPPERS`
* :prop_tgt:`<LANG>_COMPILER_LAUNCHER`
* :prop_tgt:`LINK_WRAPPERS`
