This variable controls whether the :variable:`CMAKE_FIND_ROOT_PATH` and
:variable:`CMAKE_SYSROOT` are used by |FIND_XXX|.

If set to ``ONLY``, then only the roots in :variable:`CMAKE_FIND_ROOT_PATH`
and :variable:`CMAKE_SYSROOT`, and directories below
:variable:`CMAKE_STAGING_PREFIX`, will be searched. If set to ``NEVER``, then
those roots will be ignored and only the host system root will be used.
If set to ``BOTH``, then the host system paths and the paths in those roots
will be searched.

If this variable is not set, ``BOTH`` is used. Toolchain files or
platform-specific initialization may set a different value. Explicit
root path options given to |FIND_XXX| take precedence over this variable.
