SET( SRCS 
  CMakeSetup.h
  CMakeSetup.cpp
  MakeHelp.h
  MakeHelp.cpp
  CMakeGenDialog.h
  CMakeGenDialog.cpp
  CMakeSetup.rc
  CMakeSetupDialog.h
  CMakeSetupDialog.cpp
  PathDialog.h
  PathDialog.cpp
  PropertyList.cpp
  StdAfx.h
  StdAfx.cpp
  resource.h
  CMakeCommandLineInfo.cpp
)

# add stuff to use MFC in this executable
ADD_DEFINITIONS(-D_AFXDLL)

# Use of CMAKE_MFC_FLAG 
# Values: 
# 0: Use Standard Windows Libraries 
# 1: Use MFC in a Static Library 
# 2: Use MFC in a Shared DLL 
SET(CMAKE_MFC_FLAG 2)

ADD_EXECUTABLE(CMakeSetup WIN32 ${SRCS})
TARGET_LINK_LIBRARIES(CMakeSetup CMakeLib)
ADD_DEPENDENCIES(CMakeSetup cmake)


# some older versions of cmake do not have
# a patch version set so we need to set it
# so that a later if will not fail, this has
# to be done all the time for all generators
# since if statements are evaluated even if
# inside a false if, they must still parse so for
# if (CMAKE_PATCH_VERSION GREATER 4) not to
# fail CMAKE_PATCH_VERSION must be set
IF(${CMAKE_MAJOR_VERSION} LESS 3)
  IF(NOT DEFINED CMAKE_PATCH_VERSION)
    SET(CMAKE_PATCH_VERSION 0)
  ENDIF(NOT DEFINED CMAKE_PATCH_VERSION)
ENDIF(${CMAKE_MAJOR_VERSION} LESS 3)

# for MSVC greater than 71 assume mt is around
# for other versions allow for the cache to be
# seeded with a value for CMAKE_MT_EXECUTABLE
# this allows for MSVC71 to build CMakeSetup that
# will have the manifest stuff for windows vista
IF(MSVC AND NOT MSVC60 AND NOT MSVC70 AND NOT MSVC71)
  SET(CMAKE_MT_EXECUTABLE mt)
ENDIF(MSVC AND NOT MSVC60 AND NOT MSVC70 AND NOT MSVC71)
IF(CMAKE_MT_EXECUTABLE)
  SET(exe "${CMAKE_CFG_INTDIR}/CMakeSetup.exe")
  IF(EXECUTABLE_OUTPUT_PATH)
    SET(exe "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/CMakeSetup.exe")
  ENDIF(EXECUTABLE_OUTPUT_PATH)

  # VERBATIM flag should be avoided if building with Visual Studio.
  # (OK to add the flag if ${CMAKE_CFG_INTDIR} is "." which is one
  # indicator that you are not building with Visual Studio.)
  #
  SET(verbatim_flag)
  IF(NOT CMAKE_CONFIGURATION_TYPES)
    # VERBATIM flag should also be avoided if using an older CMake.
    # VERBATIM flag support was added in CMake 2.4.5
    #
    IF(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} EQUAL 2.4)
      IF(${CMAKE_PATCH_VERSION} GREATER 4)
        SET(verbatim_flag "VERBATIM")
      ENDIF(${CMAKE_PATCH_VERSION} GREATER 4)
    ENDIF(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} EQUAL 2.4)

    IF(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.4)
      SET(verbatim_flag "VERBATIM")
    ENDIF(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 2.4)

    IF(NOT verbatim_flag)
      MESSAGE("${CMAKE_CURRENT_LIST_FILE}(${CMAKE_CURRENT_LIST_LINE}): warning
        warning: The VERBATIM flag for ADD_CUSTOM_COMMAND may be required to execute mt correctly,
                 but you are using a CMake ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} which does not parse the VERBATIM flag.
                 Upgrade to CMake 2.4.5 or later to eliminate this warning.")
    ENDIF(NOT verbatim_flag)
  ENDIF(NOT CMAKE_CONFIGURATION_TYPES)

  SET(_CMAKE_INPUT_RESOURCE "-inputresource:${exe};#1")
  # if msvc71 then you can not replace the resource
  # but you can add one, so set the input resource to empty
  IF(MSVC71)
    SET(_CMAKE_INPUT_RESOURCE )
  ENDIF(MSVC71)
  # Solve the "things named like *Setup prompt for admin privileges
  # on Vista" problem by merging a manifest fragment that contains a
  # requestedExecutionLevel element:
  #
  ADD_CUSTOM_COMMAND(TARGET CMakeSetup
    POST_BUILD COMMAND mt
    "${_CMAKE_INPUT_RESOURCE}"
    -manifest "${CMAKE_CURRENT_SOURCE_DIR}/CMakeSetupManifest.xml"
    "-outputresource:${exe};#1"
    ${verbatim_flag}
  )
ENDIF(CMAKE_MT_EXECUTABLE)

INSTALL_TARGETS(/bin CMakeSetup)
