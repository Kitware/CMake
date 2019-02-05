Xcode
-----

Generate Xcode project files.

This supports Xcode 3.0 and above.  Support for Xcode versions prior
to Xcode 5 is deprecated and will be dropped in a future version of
CMake.

Toolset Selection
^^^^^^^^^^^^^^^^^

By default Xcode is allowed to select its own default toolchain.
The :variable:`CMAKE_GENERATOR_TOOLSET` option may be set, perhaps
via the :manual:`cmake(1)` ``-T`` option, to specify another toolset.
