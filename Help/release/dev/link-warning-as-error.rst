link-warning-as-error
---------------------

* The :variable:`CMAKE_LINK_WARNING_AS_ERROR` variable and corresponding
  :prop_tgt:`LINK_WARNING_AS_ERROR` target property were added to enable
  link with a linker-specific flag to treat warnings as errors.
* The :manual:`cmake(1)` command line gained the
  :option:`--link-no-warning-as-error <cmake --link-no-warning-as-error>`
  option which causes the effects of the :prop_tgt:`LINK_WARNING_AS_ERROR`
  target property and :variable:`CMAKE_LINK_WARNING_AS_ERROR` variable to be
  ignored.
