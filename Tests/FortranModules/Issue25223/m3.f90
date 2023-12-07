module m3

use m2, only : twopi

implicit none

contains

pure real function fourpi()
fourpi = 2*twopi()
end function

end module
