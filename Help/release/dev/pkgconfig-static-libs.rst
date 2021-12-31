pkgconfig-static-libs
---------------------

* The :module:`FindPkgConfig` module learned to find static libraries
  in addition to the default search for shared libraries.
  :command:`pkg_check_modules` gained a ``STATIC_TARGET`` option
  to make the imported target reference static libraries.
