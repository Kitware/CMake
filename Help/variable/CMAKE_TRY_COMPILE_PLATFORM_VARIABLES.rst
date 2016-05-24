CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
------------------------------------

List of variables that the :command:`try_compile` command source file signature
must propagate into the test project in order to target the same platform as
the host project.

This variable should not be set by project code.  It is meant to be set by
CMake's platform information modules for the current toolchain, or by a
toolchain file when used with :variable:`CMAKE_TOOLCHAIN_FILE`.
