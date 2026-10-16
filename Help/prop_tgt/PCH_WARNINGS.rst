PCH_WARNINGS
------------

.. versionadded:: 4.6

Enable compiler warnings for the headers listed in :prop_tgt:`PRECOMPILE_HEADERS`.

By default CMake marks the generated precompiled header file as a system
header, which suppresses the diagnostics of every header it includes.  This
means that enabling precompiled headers silently hides warnings that the same
build would produce without them.

Set this property to true to omit the system header marker so that the headers
collected into the precompiled header are diagnosed as they would be in a
build without precompiled headers.  The diagnostics are reported once, while
compiling the precompiled header itself, rather than once per source file.

This property is initialized by the value of the :variable:`CMAKE_PCH_WARNINGS`
variable when the target is created.

The :generator:`Xcode` generator never adds the system header marker, so this
property has no effect there.

Targets that reuse a precompiled header through
:prop_tgt:`PRECOMPILE_HEADERS_REUSE_FROM` do not generate a precompiled header
of their own, so this property has no effect on them either.  Set it on the
target named by :prop_tgt:`PRECOMPILE_HEADERS_REUSE_FROM` instead.
