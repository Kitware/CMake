FindVTK
-------

* The :module:`FindVTK` module dropped support for finding VTK 4.0.
  It is now a thin-wrapper around ``find_package(VTK ... NO_MODULE)``.
  This produces much clearer error messages when VTK is not found.
