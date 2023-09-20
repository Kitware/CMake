module m2

use m1, only : pi

implicit none

contains

pure real function twopi()
twopi = 2*pi()
end function

end module
