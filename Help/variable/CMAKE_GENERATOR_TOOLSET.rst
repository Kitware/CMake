CMAKE_GENERATOR_TOOLSET
-----------------------

Native build system toolset specification provided by user.

Some CMake generators support a toolset specification to tell the
native build system how to choose a compiler.  If the user specifies
a toolset (e.g.  via the :manual:`cmake(1)` ``-T`` option) the value
will be available in this variable.

The value of this variable should never be modified by project code.
A toolchain file specified by the :variable:`CMAKE_TOOLCHAIN_FILE`
variable may initialize ``CMAKE_GENERATOR_TOOLSET``.  Once a given
build tree has been initialized with a particular value for this
variable, changing the value has undefined behavior.

Toolset specification is supported only on specific generators:

* :ref:`Visual Studio Generators` for VS 2010 and above
* The :generator:`Xcode` generator for Xcode 3.0 and above

See native build system documentation for allowed toolset names.

Visual Studio Toolset Selection
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

On :ref:`Visual Studio Generators` the selected toolset name
is provided in the :variable:`CMAKE_VS_PLATFORM_TOOLSET` variable.
