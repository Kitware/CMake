LINK_WRAPPERS
-------------

.. versionadded:: 4.5

List of wrapper names to be prepended to the link commands for a target's
output artifact.

This property holds a :ref:`semicolon-separated list <CMake Language Lists>`
of names, where each item is a ``<NAME>`` keyed to the
:prop_gbl:`CMAKE_LINK_WRAPPER_<NAME>` global property.

.. note::

  If a target has multiple wrappers, either via its ``LINK_WRAPPERS`` and/or
  the :prop_tgt:`INTERFACE_LINK_WRAPPERS` inherited from its dependents, the
  wrappers will be deduplicated and ordered lexicographically based on their
  name in the resulting link command(s).

This property is supported only by :ref:`Makefile Generators` and
:ref:`Ninja Generators`, and is ignored by other generators. Further, it is
supported only when the target's language is one of: ``C``, ``CXX``, ``CUDA``,
``OBJC``, ``OBJCXX``, ``Fortran``, ``HIP``.

Contents of ``LINK_WRAPPERS`` may use "generator expressions" with the syntax
``$<...>``.  See the :manual:`cmake-generator-expressions(7)` manual for
available expressions.  See the :manual:`cmake-buildsystem(7)` manual for more
on defining buildsystem properties.

Wrappers apply to the link command of executables, shared libraries, and
module libraries. As with :prop_tgt:`<LANG>_LINKER_LAUNCHER`, they do not
apply to ``STATIC`` libraries, which are created by the archiver rather than
the linker.

Interaction With Launchers
^^^^^^^^^^^^^^^^^^^^^^^^^^

Wrappers are placed *outside* of any :prop_tgt:`<LANG>_LINKER_LAUNCHER`.
The resulting link command is:

.. code-block:: shell

  <wrappers...> <launcher...> <linker> <arguments...>

The launcher used by :manual:`ctest(1)` (see :variable:`CTEST_USE_LAUNCHERS`)
remains outermost and runs the wrappers as part of the command it launches.

See Also
^^^^^^^^

Related properties:

* :prop_tgt:`COMPILE_WRAPPERS`
* :prop_tgt:`INTERFACE_LINK_WRAPPERS`
* :prop_tgt:`<LANG>_LINKER_LAUNCHER`
