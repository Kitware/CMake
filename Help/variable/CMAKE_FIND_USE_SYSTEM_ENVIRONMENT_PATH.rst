CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH
--------------------------------------

Controls the searching the standard system environment variables by the
:command:`find_program`, :command:`find_library`, :command:`find_file`,
:command:`find_path`, and command:`find_package` commands.
This is useful in cross-compiling environments.

By default this variable is not set, which is equivalent to it having
a value of ``TRUE``.  Explicit options given to the :command:`find_program`,
:command:`find_library`, :command:`find_file`, and :command:`find_path`
commands take precedence over this variable.

See also the :variable:`CMAKE_FIND_USE_CMAKE_PATH`,
:variable:`CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH`,
:variable:`CMAKE_FIND_USE_CMAKE_SYSTEM_PATH`,
:variable:`CMAKE_FIND_USE_PACKAGE_REGISTRY`,
and :variable:`CMAKE_FIND_USE_PACKAGE_ROOT_PATH` variables.
