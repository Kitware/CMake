program main
  use parent, only : child_function,grandchild_subroutine
  use parent, only : sibling_function,GreatGrandChild_subroutine
  ! Using a module without postfix
  use obfuscated_parent
  implicit none
  if (child_function())     call grandchild_subroutine
  if (sibling_function())   call GreatGrandChild_subroutine
  if (child_function_obf()) call grandchild_subroutine_obf
end program
