module parent
  implicit none

  interface

    ! Test Fortran 2008 "module function" syntax
    module function child_function() result(child_stuff)
      logical :: child_stuff
    end function

    ! Test Fortran 2008 "module subroutine" syntax
    module subroutine grandchild_subroutine()
    end subroutine

  end interface

end module parent
