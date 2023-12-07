find-rustc-importlibs
---------------------

* On Windows, when targeting the MSVC ABI, the :command:`find_library` command
  now considers ``.dll.lib`` file names before ``.lib``.  This is the default
  suffix for DLL import libraries created by Rust toolchains for the MSVC ABI.
