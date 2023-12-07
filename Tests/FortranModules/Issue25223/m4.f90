module m4

use m3, only : fourpi

implicit none

contains

pure real function halfpi()
halfpi = fourpi() / 8.0
end function

end module
