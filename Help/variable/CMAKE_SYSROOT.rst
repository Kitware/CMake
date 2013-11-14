CMAKE_SYSROOT
-------------

Path to pass to the compiler in the --sysroot flag.

The CMAKE_SYSROOT content is passed to the compiler in the --sysroot
flag, if supported.  The path is also stripped from the RPATH if
necessary on installation.  The CMAKE_SYSROOT is also used to prefix
paths searched by the ``find_*`` commands.  The CMAKE_SYSROOT is also
prefixed to the :variable:`CMAKE_INSTALL_PREFIX` so that the final
install prefix used is '${CMAKE_SYSROOT}/${CMAKE_INSTALL_PREFIX}'.  As
an alternative, the :variable:`CMAKE_STAGING_PREFIX` can be set to a location
on the host filesystem to override the final installation location.

This variable may only be set in a toolchain file. See the
:variable:`CMAKE_TOOLCHAIN_FILE` variable for details.
