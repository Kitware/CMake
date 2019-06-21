CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH
-------------------------------------

Controls the searching the cmake-specific environment variables by the :command:`find_program`,
:command:`find_library`, :command:`find_file`, :command:`find_package`, and :command:`find_path`.
This is useful in cross-compiling environments.

By default this this is set to ``TRUE``.

See also the :variable:`CMAKE_FIND_USE_CMAKE_PATH`, :variable:`CMAKE_FIND_USE_CMAKE_SYSTEM_PATH`,
:variable:`CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH`, and :variable:`CMAKE_FIND_USE_PACAKGE_ROOT_PATH` variables.
