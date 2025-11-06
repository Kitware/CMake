cmake_minimum_required(VERSION 4.0)

include(Setup.cmake)

set(CMAKE_FIND_PACKAGE_SORT_ORDER NAME)
set(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

###############################################################################
# Test reporting when trying to read a .cps that is not for the package
# requested. This is somewhat unlikely to occur in practice, since the most
# likely scenario would be searching for e.g. "a-b" and finding an appendix to
# a package named "a". However, we're likely to fail prefix resolution in that
# case, unless the appendix superfluously provides a "prefix" or "cps_path",
# and reject the file before getting as far as the name check.
find_package(WrongName REQUIRED)
