pch-warnings
------------

* The :prop_tgt:`PCH_WARNINGS` target property, and its initializing
  :variable:`CMAKE_PCH_WARNINGS` variable, were added to enable compiler
  warnings for the headers collected into a precompiled header.  Previously
  the generated precompiled header was always marked as a system header,
  which suppressed those warnings.

  The system header marker now comes from an internal
  ``CMAKE_PCH_SYSTEM_HEADER_PRAGMA`` variable instead of from
  ``CMAKE_PCH_PROLOGUE``.  Projects that cleared or replaced
  ``CMAKE_PCH_PROLOGUE`` in order to drop the marker no longer affect it,
  and must set :prop_tgt:`PCH_WARNINGS` or :variable:`CMAKE_PCH_WARNINGS`
  instead.
