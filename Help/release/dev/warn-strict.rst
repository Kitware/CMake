warn-strict
-----------

* The :diagnostic:`CMD_NON_TARGET_DIRECTIVE` diagnostic category was added to
  diagnose commands that affect the directory scope when comparable
  target-specific commands exist. The use of such directives is disrecommended.
  This diagnostic may be controlled with the
  :option:`-Wnon-target-directive <cmake -W>` command-line option,
  the ``nonTargetDirective`` field in a
  :manual:`CMake Presets <cmake-presets(7)>`
  :preset:`configurePresets.warnings` object,
  or the :command:`cmake_diagnostic` command.

* The :diagnostic:`CMD_STRICT` diagnostic category was added to control
  diagnostics regarding allowed but disrecommended usage as a group.
  This diagnostic may be controlled with the :option:`-Wstrict <cmake -W>`
  command-line option, the ``strict`` field in a
  :manual:`CMake Presets <cmake-presets(7)>`
  :preset:`configurePresets.warnings` object,
  or the :command:`cmake_diagnostic` command.
