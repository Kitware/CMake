presets-fileDir
---------------

* :manual:`cmake-presets(7)` files now support schema version 12. The
  :ref:`${fileDir} <CMakePresets fileDir>` macro now always expands to the
  directory of the preset file containing the ``${fileDir}`` macro, regardless
  of whether it is inherited by another preset in a different directory.
