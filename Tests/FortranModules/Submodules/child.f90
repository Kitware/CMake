! Test the notation for a 1st-generation direct
! descendant of a parent module
submodule ( parent ) child
  implicit none
contains
  module function child_function() result(child_stuff)
    logical :: child_stuff
    child_stuff=.true.
  end function
end submodule child
