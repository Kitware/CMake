# Find KDE4 and provide all necessary variables and macros to compile software for it.
# It looks for KDE 4 in the following directories in the given order:
# - CMAKE_INSTALL_PREFIX
# - KDEDIRS
# - /opt/kde4
# - /opt/kde
#
# Please look in FindKDE4Internal.cmake and KDE4Macros.cmake for more information.
# They are installed with the KDE 4 libraries in $KDEDIRS/share/apps/cmake/modules/.
#
# Author: Alexander Neundorf <neundorf@kde.org>

FILE(TO_CMAKE_PATH "$ENV{KDEDIRS}" _KDEDIRS)

# For KDE4 kde-config has been renamed to kde4-config
FIND_PROGRAM(KDE4_KDECONFIG_EXECUTABLE NAMES kde4-config 
   PATH_SUFFIXES bin    # the suffix is for the paths coming from KDEDIRS
   PATHS
   ${CMAKE_INSTALL_PREFIX}/bin
   ${_KDEDIRS}
   /opt/kde4/bin
   NO_DEFAULT_PATH
   )


FIND_PROGRAM(KDE4_KDECONFIG_EXECUTABLE NAMES kde4-config )

IF (KDE4_KDECONFIG_EXECUTABLE)
   # then ask kde4-config for the kde data dirs
   EXEC_PROGRAM(${KDE4_KDECONFIG_EXECUTABLE} ARGS --path data OUTPUT_VARIABLE _data_DIR )

   FILE(TO_CMAKE_PATH "${_data_DIR}" _data_DIR)

   # then check the data dirs for FindKDE4Internal.cmake
   FIND_PATH(KDE4_DATA_DIR cmake/modules/FindKDE4Internal.cmake ${_data_DIR})

   # if it has been found...
   IF (KDE4_DATA_DIR)

      SET(CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH} ${KDE4_DATA_DIR}/cmake/modules)

      IF (KDE4_FIND_QUIETLY)
         SET(_quiet QUIET)
      ENDIF (KDE4_FIND_QUIETLY)

      IF (KDE4_FIND_REQUIRED)
         SET(_req REQUIRED)
      ENDIF (KDE4_FIND_REQUIRED)

      # use FindKDE4Internal.cmake to do the rest
      FIND_PACKAGE(KDE4Internal ${_req} ${_quiet})
   ELSE (KDE4_DATA_DIR)
      IF (KDE4_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "ERROR: cmake/modules/FindKDE4Internal.cmake not found in ${_data_DIR}")
      ENDIF (KDE4_FIND_REQUIRED)
   ENDIF (KDE4_DATA_DIR)

ELSE (KDE4_KDECONFIG_EXECUTABLE)
   IF (KDE4_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "ERROR: Could not find KDE4 kde4-config")
   ENDIF (KDE4_FIND_REQUIRED)
ENDIF (KDE4_KDECONFIG_EXECUTABLE)

