#
# This module finds if Python is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  PYTHON_LIBRARIES       = the full path to the library found
#  PYTHON_INCLUDE_PATH    = the path to where Python.h can be found
#  PYTHON_DEBUG_LIBRARIES = the full path to the debug library found
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
  NAMES python23 python2.3 python2.3.dll
        python22 python2.2 python2.2.dll
        python21 python2.1 python2.1.dll
        python20 python2.0 python2.0.dll
        python16 python1.6 python1.6.dll
        python15 python1.5 python1.5.dll
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
  ~/Library/Frameworks/Python.framework/Headers
  /Library/Frameworks/Python.framework/Headers
  ~/Library/Frameworks/Python.framework/Versions/2.3/include/python2.3
  /Library/Frameworks/Python.framework/Versions/2.3/include/python2.3
  ~/Library/Frameworks/Python.framework/Versions/2.2/include/python2.2
  /Library/Frameworks/Python.framework/Versions/2.2/include/python2.2
  ~/Library/Frameworks/Python.framework/Versions/2.1/include/python2.1
  /Library/Frameworks/Python.framework/Versions/2.1/include/python2.1
  ~/Library/Frameworks/Python.framework/Versions/2.0/include/python2.0
  /Library/Frameworks/Python.framework/Versions/2.0/include/python2.0
  ~/Library/Frameworks/Python.framework/Versions/1.6/include/python1.6
  /Library/Frameworks/Python.framework/Versions/1.6/include/python1.6
  ~/Library/Frameworks/Python.framework/Versions/1.5/include/python1.5
  /Library/Frameworks/Python.framework/Versions/1.5/include/python1.5
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

# Python Should be built and installed as a Framework on OSX
IF (APPLE)
  IF(EXISTS ~/Library/Frameworks/Python.framework)
    SET(PYTHON_HAVE_FRAMEWORK 1)
  ENDIF(EXISTS ~/Library/Frameworks/Python.framework)
  IF(EXISTS /Library/Frameworks/Python.framework)
    SET(PYTHON_HAVE_FRAMEWORK 1)
  ENDIF(EXISTS /Library/Frameworks/Python.framework)
  IF("${PYTHON_INCLUDE_PATH}" MATCHES "Python\\.framework")
    SET(PYTHON_LIBRARY "")
    SET(PYTHON_DEBUG_LIBRARY "")
  ENDIF("${PYTHON_INCLUDE_PATH}" MATCHES "Python\\.framework")
  IF(PYTHON_HAVE_FRAMEWORK)
    IF(NOT PYTHON_LIBRARY)
      SET (PYTHON_LIBRARY "-framework Python" CACHE FILEPATH "Python Framework" FORCE)
    ENDIF(NOT PYTHON_LIBRARY)
    IF(NOT PYTHON_DEBUG_LIBRARY)
      SET (PYTHON_DEBUG_LIBRARY "-framework Python" CACHE FILEPATH "Python Framework" FORCE)
    ENDIF(NOT PYTHON_DEBUG_LIBRARY)
  ENDIF(PYTHON_HAVE_FRAMEWORK)
ENDIF (APPLE)

# We use PYTHON_LIBRARY and PYTHON_DEBUG_LIBRARY for the cache entries
# because they are meant to specify the location of a single library.
# We now set the variables listed by the documentation for this
# module.
SET(PYTHON_LIBRARIES "${PYTHON_LIBRARY}")
SET(PYTHON_DEBUG_LIBRARIES "${PYTHON_DEBUG_LIBRARY}")
