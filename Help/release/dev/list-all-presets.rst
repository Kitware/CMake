list-all-presets
----------------

* :option:`cmake --list-presets <cmake --list-presets>` gained
  ``*-defined`` values that list all non-hidden presets of the corresponding
  type and explain why unavailable presets cannot be used. The ``defined``
  value is an alias for ``configure-defined``, while ``all-defined`` selects
  every type. Build, test, and package presets whose configure preset is
  unavailable are now considered unavailable too.

* :option:`cmake --workflow --list-presets=defined
  <cmake--workflow --list-presets>` was added to provide the same
  availability information for all defined workflow presets and their steps.

* :option:`cmake --build --list-presets=defined
  <cmake--build --list-presets>`,
  :option:`ctest --list-presets=defined <ctest --list-presets>`, and
  :option:`cpack --list-presets=defined <cpack --list-presets>` were added to
  list all defined build, test, and package presets, respectively.
