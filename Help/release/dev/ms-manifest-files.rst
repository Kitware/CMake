ms-manifest-files
-----------------

* CMake learned to honor ``*.manifest`` source files with MSVC tools.
  Manifest files named as sources of ``.exe`` and ``.dll`` targets
  will be merged with linker-generated manifests and embedded in the
  binary.
