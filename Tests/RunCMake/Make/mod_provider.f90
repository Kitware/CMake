module mymod
  implicit none
contains
  function modval() result(v)
    integer :: v
    v = 42
  end function modval
end module mymod
