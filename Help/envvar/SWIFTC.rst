SWIFTC
------

.. include:: ENV_VAR.txt

Preferred executable for compiling ``Swift`` language files. Will only be used by
CMake on the first configuration to determine ``Swift`` compiler, after which the
value for ``SWIFTC`` is stored in the cache as
:variable:`CMAKE_Swift_COMPILER <CMAKE_<LANG>_COMPILER>`. For any configuration run
(including the first), the environment variable will be ignored if the
:variable:`CMAKE_Swift_COMPILER <CMAKE_<LANG>_COMPILER>` variable is defined.
