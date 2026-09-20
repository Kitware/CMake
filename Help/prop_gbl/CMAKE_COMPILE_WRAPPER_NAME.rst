CMAKE_COMPILE_WRAPPER_<NAME>
----------------------------

.. versionadded:: 4.5

Command line to be prepended to compile commands.

This property holds a command line to be used with the
:prop_tgt:`COMPILE_WRAPPERS` and/or :prop_tgt:`INTERFACE_COMPILE_WRAPPERS`
target properties. Named values in those target properties (which are
:ref:`semicolon-separated lists <CMake Language Lists>`) will reference the
correspondingly-named ``CMAKE_COMPILE_WRAPPER_<NAME>`` and prepend the compile
command(s) for source(s) of the given target or its consumer(s) with the value
of this property.

This property is supported only by :ref:`Makefile Generators` and
:ref:`Ninja Generators`, and is ignored by other generators.

The value is a :ref:`semicolon-separated list <CMake Language Lists>` of
command line arguments.  Empty arguments are preserved, and a literal
semicolon within an argument may be written as ``\;``.

Contents of ``CMAKE_COMPILE_WRAPPER_<NAME>`` may use "generator expressions"
with the syntax ``$<...>``.  See the :manual:`cmake-generator-expressions(7)`
manual for available expressions.  See the :manual:`cmake-buildsystem(7)`
manual for more on defining buildsystem properties.

Example
^^^^^^^

Consider the following example:

.. code-block:: cmake

  add_library(iface INTERFACE)
  add_executable(main main.c)
  target_link_libraries(main PRIVATE iface)

  set_property(GLOBAL
    PROPERTY CMAKE_COMPILE_WRAPPER_2_PYTHON_WRAPPER
      "${Python_EXECUTABLE}" my_wrapper.py
  )
  set_property(GLOBAL
    PROPERTY CMAKE_COMPILE_WRAPPER_1_TIME
      "${CMAKE_COMMAND}" -E time
  )

  set_target_properties(iface PROPERTIES
    INTERFACE_COMPILE_WRAPPERS 1_TIME
  )
  set_target_properties(main PROPERTIES
    COMPILE_WRAPPERS 2_PYTHON_WRAPPER
  )

Here, all compile commands for the sources of ``main`` will look like:

.. code-block:: shell

  /path/to/cmake -E time /path/to/python my_wrapper.py ...

See Also
^^^^^^^^

* :prop_gbl:`CMAKE_LINK_WRAPPER_<NAME>`
* :prop_tgt:`COMPILE_WRAPPERS`
* :prop_tgt:`INTERFACE_COMPILE_WRAPPERS`
* :prop_tgt:`<LANG>_COMPILER_LAUNCHER`
