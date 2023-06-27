Visual Studio 12 2013
---------------------

Deprecated.  Generates Visual Studio 12 (VS 2013) project files.

.. note::
  This generator is deprecated and will be removed in a future version
  of CMake.  It will still be possible to build with VS 12 2013 tools
  using the :generator:`Visual Studio 14 2015` (or above) generator
  with :variable:`CMAKE_GENERATOR_TOOLSET` set to ``v120``, or by
  using the :generator:`NMake Makefiles` generator.

For compatibility with CMake versions prior to 3.0, one may specify this
generator using the name "Visual Studio 12" without the year component.

Project Types
^^^^^^^^^^^^^

Only Visual C++ and C# projects may be generated (and Fortran with
Intel compiler integration).  Other types of projects (JavaScript,
Powershell, Python, etc.) are not supported.

Platform Selection
^^^^^^^^^^^^^^^^^^

The default target platform name (architecture) is ``Win32``.

.. versionadded:: 3.1
  The :variable:`CMAKE_GENERATOR_PLATFORM` variable may be set, perhaps
  via the :option:`cmake -A` option, to specify a target platform
  name (architecture).  For example:

  * ``cmake -G "Visual Studio 12 2013" -A Win32``
  * ``cmake -G "Visual Studio 12 2013" -A x64``
  * ``cmake -G "Visual Studio 12 2013" -A ARM``

For compatibility with CMake versions prior to 3.1, one may specify
a target platform name optionally at the end of the generator name.
This is supported only for:

``Visual Studio 12 2013 Win64``
  Specify target platform ``x64``.

``Visual Studio 12 2013 ARM``
  Specify target platform ``ARM``.

Toolset Selection
^^^^^^^^^^^^^^^^^

The ``v120`` toolset that comes with Visual Studio 12 2013 is selected by
default.  The :variable:`CMAKE_GENERATOR_TOOLSET` option may be set, perhaps
via the :option:`cmake -T` option, to specify another toolset.

.. |VS_TOOLSET_HOST_ARCH_DEFAULT| replace::
   By default this generator uses the 32-bit variant even on a 64-bit host.

.. include:: VS_TOOLSET_HOST_ARCH_LEGACY.txt
