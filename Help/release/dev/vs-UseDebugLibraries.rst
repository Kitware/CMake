vs-UseDebugLibraries
--------------------

* :ref:`Visual Studio Generators` now add ``UseDebugLibraries`` indicators to
  ``.vcxproj`` files to denote which configurations are debug configurations.
  See policy :policy:`CMP0162`.

* The :variable:`CMAKE_VS_USE_DEBUG_LIBRARIES` variable and corresponding
  :prop_tgt:`VS_USE_DEBUG_LIBRARIES` target property were added to explicitly
  control ``UseDebugLibraries`` indicators in ``.vcxproj`` files.
