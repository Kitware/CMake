Visual Studio 9 2008
--------------------

Deprecated.  Generates Visual Studio 9 2008 project files.

.. note::
  This generator is deprecated and will be removed in a future version
  of CMake.  It will still be possible to build with VS 9 2008 tools
  using the :generator:`Visual Studio 12 2013` generator (or above,
  and with VS 10 2010 also installed) with
  :variable:`CMAKE_GENERATOR_TOOLSET` set to ``v90``,
  or by using the :generator:`NMake Makefiles` generator.

Platform Selection
^^^^^^^^^^^^^^^^^^

The default target platform name (architecture) is ``Win32``.

.. versionadded:: 3.1
  The :variable:`CMAKE_GENERATOR_PLATFORM` variable may be set, perhaps
  via the :option:`cmake -A` option, to specify a target platform
  name (architecture).  For example:

  * ``cmake -G "Visual Studio 9 2008" -A Win32``
  * ``cmake -G "Visual Studio 9 2008" -A x64``
  * ``cmake -G "Visual Studio 9 2008" -A Itanium``
  * ``cmake -G "Visual Studio 9 2008" -A <WinCE-SDK>``
    (Specify a target platform matching a Windows CE SDK name.)

For compatibility with CMake versions prior to 3.1, one may specify
a target platform name optionally at the end of the generator name.
This is supported only for:

``Visual Studio 9 2008 Win64``
  Specify target platform ``x64``.

``Visual Studio 9 2008 IA64``
  Specify target platform ``Itanium``.

``Visual Studio 9 2008 <WinCE-SDK>``
  Specify target platform matching a Windows CE SDK name.
