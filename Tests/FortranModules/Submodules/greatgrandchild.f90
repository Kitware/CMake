! Test the notation for an Nth-generation descendant
! for N>1, which necessitates the colon.
! LLVMFlang's preprocessed output splits this long name across lines.
submodule ( parent : grandchild ) GreatGrandChild_With_A_Name_Long_Enough_To_Wrap
contains
  module subroutine GreatGrandChild_subroutine()
    print *,"Test passed."
  end subroutine
end submodule GreatGrandChild_With_A_Name_Long_Enough_To_Wrap
