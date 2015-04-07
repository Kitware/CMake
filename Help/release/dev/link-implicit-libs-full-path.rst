link-implicit-libs-full-path
----------------------------

* Linking to library files by a full path in an implicit linker search
  directory (e.g. ``/usr/lib/libfoo.a``) no longer asks the linker to
  search for the library (e.g. ``-lfoo``).  See policy :policy:`CMP0060`.
