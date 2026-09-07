#define HAVE_MPI 1
#ifdef HAVE_MPI
      use mpi
#else
      use serial_stub
#endif
      include 'missing.inc'
