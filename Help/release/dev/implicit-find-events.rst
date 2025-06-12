implicit-find-events
--------------------

* The :manual:`cmake-configure-log(7)` will report events from ``find_``
  commands without any find-debug flags (e.g.,
  :variable:`CMAKE_FIND_DEBUG_MODE`) when they transition between "found" and
  "not found" or when they are first defined. The
  :variable:`CMAKE_FIND_DEBUG_MODE_NO_IMPLICIT_CONFIGURE_LOG` variable will
  suppress these events without any explicit request for a debug mode.
