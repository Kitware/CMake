CMAKE_SYSTEM_VERSION
--------------------

The version of the operating system for which CMake is to build.
See the :variable:`CMAKE_SYSTEM_NAME` variable for the OS name.

System Version for Host Builds
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When the :variable:`CMAKE_SYSTEM_NAME` variable takes its default value
then ``CMAKE_SYSTEM_VERSION`` is by default set to the same value as the
:variable:`CMAKE_HOST_SYSTEM_VERSION` variable so that the build targets
the host system version.

System Version for Cross Compiling
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When the :variable:`CMAKE_SYSTEM_NAME` variable is set explicitly to
enable :ref:`cross compiling <Cross Compiling Toolchain>` then the
value of ``CMAKE_SYSTEM_VERSION`` must also be set explicitly to specify
the target system version.
