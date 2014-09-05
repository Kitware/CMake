CMAKE_GENERATOR_PLATFORM
------------------------

Generator-specific target platform name specified by user.

Some CMake generators support a target platform name to be given
to the native build system to choose a compiler toolchain.

The value of this variable should never be modified by project code.
A toolchain file specified by the :variable:`CMAKE_TOOLCHAIN_FILE`
variable may initialize ``CMAKE_GENERATOR_PLATFORM``.  Once a given
build tree has been initialized with a particular value for this
variable, changing the value has undefined behavior.
