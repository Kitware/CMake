xcode-link-phase-all
--------------------

* The Xcode generator gained support for linking libraries and frameworks
  via the *Link Binaries With Libraries* build phase instead of always by
  embedding linker flags directly.  This behavior is controlled by a new
  :prop_tgt:`XCODE_LINK_BUILD_PHASE_MODE` target property, which is
  initialized by a new :variable:`CMAKE_XCODE_LINK_BUILD_PHASE_MODE`
  variable.
