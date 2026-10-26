fortran-subdir-build-config
---------------------------

* The :module:`CMakeAddFortranSubdirectory` module now builds the MinGW
  Fortran project with :variable:`CMAKE_BUILD_TYPE` set to the configuration
  being built by the parent project.  See policy :policy:`CMP0230`.
