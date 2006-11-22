# - obsolete pkg-config module for CMake
#
# Includes FindPkgConfig.cmake which defines 
#
# PKGCONFIG(package includedir libdir linkflags cflags)
#
# Calling PKGCONFIG will fill the desired information into the 4 given arguments,
# e.g. PKGCONFIG(libart-2.0 LIBART_INCLUDE_DIR LIBART_LINK_DIR LIBART_LINK_FLAGS LIBART_CFLAGS)
# if pkg-config was NOT found or the specified software package doesn't exist, the
# variable will be empty when the function returns, otherwise they will contain the respective information
#

INCLUDE(FindPkgConfig)
