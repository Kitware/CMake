! This module has two procedures from the "parent" module
! but it has different combinations 'module <word>' phrases
! in breaked lines for test of modules dependencies detection

! Module declaration on breaked line with reminder
module &
         obfuscated_parent;  implicit none

  interface

    ! Boolean module function
    module logical &
                     function child_function_obf() result(child_stuff)
    end function

    ! Module subroutine
    module subroutine &
                     grandchild_subroutine_obf()
    end subroutine

  end interface

  contains

    module logical function child_function_obf() result(child_stuff)
      child_stuff=.true.
    end function

    module subroutine grandchild_subroutine_obf()
      print *,"Test passed."
    end subroutine

end module obfuscated_parent
