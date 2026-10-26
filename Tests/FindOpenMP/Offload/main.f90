      program main
      implicit none
      integer, external :: offload_check
      if (offload_check() /= 0) error stop 1
      end program main
