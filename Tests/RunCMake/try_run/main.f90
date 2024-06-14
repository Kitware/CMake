program main

implicit none

interface
subroutine func() bind(C)
end subroutine
end interface

call func()

end program
