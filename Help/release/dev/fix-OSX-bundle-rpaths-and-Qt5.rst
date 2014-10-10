fix-OSX-bundle-rpaths-and-Qt5
-----------------------------

* The :module:`BundleUtilities` module learned to resolve and replace
  ``@rpath`` placeholders on OS X to correctly bundle applications
  using them.
