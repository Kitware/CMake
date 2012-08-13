# - this module looks for Matlab
# Defines:
#  MATLAB_INCLUDE_DIR: include path for mex.h, engine.h
#  MATLAB_LIBRARIES:   required libraries: libmex, etc
#  MATLAB_MEX_LIBRARY: path to libmex.lib
#  MATLAB_MX_LIBRARY:  path to libmx.lib
#  MATLAB_ENG_LIBRARY: path to libeng.lib

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

set(MATLAB_FOUND 0)
if(WIN32)
  if(${CMAKE_GENERATOR} MATCHES "Visual Studio 6")
    set(MATLAB_ROOT "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MathWorks\\MATLAB\\7.0;MATLABROOT]/extern/lib/win32/microsoft/msvc60")
  else()
    if(${CMAKE_GENERATOR} MATCHES "Visual Studio 7")
      # Assume people are generally using 7.1,
      # if using 7.0 need to link to: ../extern/lib/win32/microsoft/msvc70
      set(MATLAB_ROOT "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MathWorks\\MATLAB\\7.0;MATLABROOT]/extern/lib/win32/microsoft/msvc71")
    else()
      if(${CMAKE_GENERATOR} MATCHES "Borland")
        # Same here, there are also: bcc50 and bcc51 directories
        set(MATLAB_ROOT "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MathWorks\\MATLAB\\7.0;MATLABROOT]/extern/lib/win32/microsoft/bcc54")
      else()
        if(MATLAB_FIND_REQUIRED)
          message(FATAL_ERROR "Generator not compatible: ${CMAKE_GENERATOR}")
        endif()
      endif()
    endif()
  endif()
  find_library(MATLAB_MEX_LIBRARY
    libmex
    ${MATLAB_ROOT}
    )
  find_library(MATLAB_MX_LIBRARY
    libmx
    ${MATLAB_ROOT}
    )
  find_library(MATLAB_ENG_LIBRARY
    libeng
    ${MATLAB_ROOT}
    )

  find_path(MATLAB_INCLUDE_DIR
    "mex.h"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MathWorks\\MATLAB\\7.0;MATLABROOT]/extern/include"
    )
else()
  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # Regular x86
    set(MATLAB_ROOT
      /usr/local/matlab-7sp1/bin/glnx86/
      /opt/matlab-7sp1/bin/glnx86/
      $ENV{HOME}/matlab-7sp1/bin/glnx86/
      $ENV{HOME}/redhat-matlab/bin/glnx86/
      )
  else()
    # AMD64:
    set(MATLAB_ROOT
      /usr/local/matlab-7sp1/bin/glnxa64/
      /opt/matlab-7sp1/bin/glnxa64/
      $ENV{HOME}/matlab7_64/bin/glnxa64/
      $ENV{HOME}/matlab-7sp1/bin/glnxa64/
      $ENV{HOME}/redhat-matlab/bin/glnxa64/
      )
  endif()
  find_library(MATLAB_MEX_LIBRARY
    mex
    ${MATLAB_ROOT}
    )
  find_library(MATLAB_MX_LIBRARY
    mx
    ${MATLAB_ROOT}
    )
  find_library(MATLAB_ENG_LIBRARY
    eng
    ${MATLAB_ROOT}
    )
  find_path(MATLAB_INCLUDE_DIR
    "mex.h"
    "/usr/local/matlab-7sp1/extern/include/"
    "/opt/matlab-7sp1/extern/include/"
    "$ENV{HOME}/matlab-7sp1/extern/include/"
    "$ENV{HOME}/redhat-matlab/extern/include/"
    )

endif()

# This is common to UNIX and Win32:
set(MATLAB_LIBRARIES
  ${MATLAB_MEX_LIBRARY}
  ${MATLAB_MX_LIBRARY}
  ${MATLAB_ENG_LIBRARY}
)

if(MATLAB_INCLUDE_DIR AND MATLAB_LIBRARIES)
  set(MATLAB_FOUND 1)
endif()

mark_as_advanced(
  MATLAB_LIBRARIES
  MATLAB_MEX_LIBRARY
  MATLAB_MX_LIBRARY
  MATLAB_ENG_LIBRARY
  MATLAB_INCLUDE_DIR
  MATLAB_FOUND
  MATLAB_ROOT
)

