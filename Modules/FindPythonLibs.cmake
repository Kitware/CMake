#
# This module finds if Python is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  PYTHON_LIBRARY       = the full path to the library found
#  PYTHON_INCLUDE_PATH  = the path to where tcl.h can be found
#  PYTHON_DEBUG_LIBRARY = the full path to the debug library found
#

IF(WIN32)
  FIND_LIBRARY(PYTHON_DEBUG_LIBRARY 
    NAMES python23_d python22_d python21_d python20_d python 
    PATHS
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.3\\InstallPath]/libs/Debug
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.3\\InstallPath]/libs
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.2\\InstallPath]/libs/Debug
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.2\\InstallPath]/libs
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.1\\InstallPath]/libs/Debug
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.1\\InstallPath]/libs
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.0\\InstallPath]/libs/Debug
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.0\\InstallPath]/libs
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\1.6\\InstallPath]/libs/Debug
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\1.6\\InstallPath]/libs
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\1.5\\InstallPath]/libs/Debug
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\1.5\\InstallPath]/libs
  )
ENDIF(WIN32)

FIND_LIBRARY(PYTHON_LIBRARY 
  NAMES python23 python2.3 python22 python2.2 python21 python2.1 
        python20 python2.0 python16 python1.6 python15 python1.5
  PATHS
  /usr/lib/python2.3/config
  /usr/lib/python2.2/config
  /usr/lib/python2.1/config
  /usr/lib/python2.0/config
  /usr/lib/python1.6/config
  /usr/lib/python1.5/config
  /usr/lib
  /usr/local/lib
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.3\\InstallPath]/libs
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.2\\InstallPath]/libs
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.1\\InstallPath]/libs
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.0\\InstallPath]/libs
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\1.6\\InstallPath]/libs
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\1.5\\InstallPath]/libs
)

FIND_PATH(PYTHON_INCLUDE_PATH Python.h
  /usr/include/python2.3
  /usr/include/python2.2
  /usr/include/python2.1
  /usr/include/python2.0
  /usr/include/python1.6
  /usr/include/python1.5
  /usr/include
  /usr/local/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.3\\InstallPath]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.2\\InstallPath]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.1\\InstallPath]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\2.0\\InstallPath]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\1.6\\InstallPath]/include
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\1.5\\InstallPath]/include
)

IF (WIN32)
  MARK_AS_ADVANCED(
    PYTHON_DEBUG_LIBRARY 
    PYTHON_LIBRARY 
    PYTHON_INCLUDE_PATH
  )
ENDIF(WIN32)
