add_library-no-static-fallback
------------------------------

* On platforms that do not support shared libraries, the :command:`add_library`
  command now rejects creation of shared libraries instead of automatically
  converting them to static libraries.  See policy :policy:`CMP0164`.
