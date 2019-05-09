Swift Language Support
----------------------

* Preliminary support for the Swift language with the :generator:`Ninja`
  generator was added.  Use the :envvar:`SWIFTC` environment variable to
  specify a compiler.

* Support to emit an output file map was added to enable Swift compilation.

* A target property :prop_tgt:`Swift_DEPENDENCIES_FILE` was added to targets to
  indicate where to save the target swift dependencies file.  If one is not
  specified, it will default to `<TARGET>.swiftdeps`.

* A target property :prop_tgt:`Swift_MODULE_NAME` was added to targets to
  indicate the Swift module name.  If it is not specified, it will default to
  the name of the target.

* A source property :prop_sf:`Swift_DEPENDENCIES_FILE` was added to sources to
  indicate where to save the target swift dependencies file.  If one is not
  specified, it will default to `<OBJECT>.swiftdeps`.

* A source property :prop_sf:`Swift_DIAGNOSTICS_FILE` was added to sources to
  indicate where to write the serialised Swift diagnostics.
