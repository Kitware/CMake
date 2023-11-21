module lib

use m1, only : pi

implicit none

contains

pure real function func()
func = pi()
end function

end module
