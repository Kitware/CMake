compile-features-standard-logic-rework
--------------------------------------

* The :manual:`Compile Features <cmake-compile-features(7)>` functionality now
  correctly disables or enables compiler extensions when no standard level is
  specified and avoids unnecessarily adding language standard flags if the
  requested settings match the compiler's defaults. See :policy:`CMP0128`.

* :prop_tgt:`<LANG>_EXTENSIONS` is initialized to
  :variable:`CMAKE_<LANG>_EXTENSIONS_DEFAULT`. See :policy:`CMP0128`.
