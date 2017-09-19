findmpi-overhaul
----------------

* :module:`FindMPI` gained a number of new features, including:

  * Language-specific components have been added to the module.
  * Many more MPI environments are now supported.
  * The environmental support for Fortran has been improved.
  * A user now has fine-grained control over the MPI selection process,
    including passing custom parameters to the MPI compiler.
  * The version of the implemented MPI standard is now being exposed.
  * MPI-2 C++ bindings can now be detected and also suppressed if so desired.
  * The available Fortran bindings are now being detected and verified.
  * Various MPI-3 information can be requested, including the library version
    and Fortran capabilities of the individual bindings.
  * Statically linked MPI implementations are supported.
