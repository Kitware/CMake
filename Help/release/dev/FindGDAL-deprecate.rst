FindGDAL-deprecate
------------------

* The :module:`FindGDAL` module is now deprecated in favor of upstream
  GDAL's official CMake package configuration file. Port projects to
  the latter by calling ``find_package(GDAL CONFIG)``.  For further
  details, see `GDAL's documentation on CMake integration
  <https://gdal.org/en/latest/development/cmake.html>`_.
