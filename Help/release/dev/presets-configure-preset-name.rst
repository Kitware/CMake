presets-configure-preset-name
-----------------------------

* :manual:`cmake-presets(7)` gained the ``${configurePresetName}`` macro to
  expand to the name of the configure preset associated with the preset being
  evaluated. It is available for configure, build, test, and package presets,
  but not for workflow presets.
