# - Try to find alglib 
# Once done this will define
#  ALGLIB_INCLUDE_DIRS - The alglib include directory
#  ALGLIB_LIB - The libraries needed to use alglib
#  ALGLIB_FOUND -  True if ALGLIB found.

find_path (ALGLIB_INCLUDE_DIRS 
  ap.h
  PATHS ${CMAKE_INSTALL_PREFIX}/include
  PATH_SUFFIXES libalglib)
find_library (ALGLIB_LIB NAMES alglib)

include (FindPackageHandleStandardArgs)

find_package_handle_standard_args (ALGLIB DEFAULT_MSG ALGLIB_LIB ALGLIB_INCLUDE_DIRS)

mark_as_advanced (ALGLIB_LIB ALGLIB_INCLUDE_DIRS)
