program main

use m1, only : pi
use m4, only : halfpi

implicit none

real :: rpi, rhalfpi

rpi = pi() / 2
rhalfpi = halfpi()

print '(a,ES15.8)', 'floating point precision loss: ', rpi - rhalfpi

end program
