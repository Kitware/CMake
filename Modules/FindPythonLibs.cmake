#
# This module finds if Python is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  PYTHON_LIBRARY       = the full path to the library found
#  PYTHON_INCLUDE_PATH  = the path to where tcl.h can be found
#  PYTHON_DEBUG_LIBRARY = the full path to the debug library found
#

FIND_LIBRARY(PYTHON_DEBUG_LIBRARY 
  NAMES python python21_d python20_d
  PATHS
  /usr/lib
  /usr/local/lib
  [HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\2.1\InstallPath]/libs/Debug
  [HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\2.0\InstallPath]/libs/Debug
)

FIND_LIBRARY(PYTHON_LIBRARY 
  NAMES python python21 python20
  PATHS
  /usr/lib
  /usr/local/lib
  [HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\2.1\InstallPath]/libs
  [HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\2.0\InstallPath]/libs
)
  
FIND_PATH(PYTHON_INCLUDE_PATH Python.h
  /usr/include
  /usr/local/include
  [HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\2.1\InstallPath]/include
  [HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\2.0\InstallPath]/include
)

