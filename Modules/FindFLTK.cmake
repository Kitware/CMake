# - Find the native FLTK includes and library
# The following settings are defined
#  FLTK_FLUID_EXECUTABLE, where to find the Fluid tool
#  FLTK_WRAP_UI, This enables the FLTK_WRAP_UI command
#  FLTK_INCLUDE_DIR, where to find include files
#  FLTK_LIBRARIES, list of fltk libraries
#  FLTK_FOUND, Don't use FLTK if false.
# The following settings should not be used in general.
#  FLTK_BASE_LIBRARY   = the full path to fltk.lib
#  FLTK_GL_LIBRARY     = the full path to fltk_gl.lib
#  FLTK_FORMS_LIBRARY  = the full path to fltk_forms.lib
#  FLTK_IMAGES_LIBRARY = the full path to fltk_images.lib

#  Platform dependent libraries required by FLTK
IF(WIN32)
  IF(NOT CYGWIN)
    IF(BORLAND)
      SET( FLTK_PLATFORM_DEPENDENT_LIBS import32 )
    ELSE(BORLAND)
      SET( FLTK_PLATFORM_DEPENDENT_LIBS wsock32 comctl32 )
    ENDIF(BORLAND)
  ENDIF(NOT CYGWIN)
ENDIF(WIN32)

IF(UNIX)
  INCLUDE(${CMAKE_ROOT}/Modules/FindX11.cmake)
  SET( FLTK_PLATFORM_DEPENDENT_LIBS ${X11_LIBRARIES} -lm)
ENDIF(UNIX)

IF(APPLE)
  SET( FLTK_PLATFORM_DEPENDENT_LIBS  "-framework Carbon -framework Cocoa -framework ApplicationServices -lz")
ENDIF(APPLE)

IF(CYGWIN)
  SET( FLTK_PLATFORM_DEPENDENT_LIBS ole32 uuid comctl32 wsock32 supc++ -lm -lgdi32)
ENDIF(CYGWIN)

# If FLTK_INCLUDE_DIR is already defined we assigne its value to FLTK_DIR
IF(FLTK_INCLUDE_DIR)
  SET(FLTK_DIR ${FLTK_INCLUDE_DIR})
ENDIF(FLTK_INCLUDE_DIR)


# If FLTK has been built using CMake we try to find everything directly
SET(FLTK_DIR_STRING "directory containing FLTKConfig.cmake.  This is either the root of the build tree, or PREFIX/lib/fltk for an installation.")

# Search only if the location is not already known.
IF(NOT FLTK_DIR)
  # Get the system search path as a list.
  IF(UNIX)
    STRING(REGEX MATCHALL "[^:]+" FLTK_DIR_SEARCH1 "$ENV{PATH}")
  ELSE(UNIX)
    STRING(REGEX REPLACE "\\\\" "/" FLTK_DIR_SEARCH1 "$ENV{PATH}")
  ENDIF(UNIX)
  STRING(REGEX REPLACE "/;" ";" FLTK_DIR_SEARCH2 ${FLTK_DIR_SEARCH1})

  # Construct a set of paths relative to the system search path.
  SET(FLTK_DIR_SEARCH "")
  FOREACH(dir ${FLTK_DIR_SEARCH2})
    SET(FLTK_DIR_SEARCH ${FLTK_DIR_SEARCH} "${dir}/../lib/fltk")
  ENDFOREACH(dir)

  #
  # Look for an installation or build tree.
  #
  FIND_PATH(FLTK_DIR FLTKConfig.cmake
    # Look for an environment variable FLTK_DIR.
    $ENV{FLTK_DIR}

    # Look in places relative to the system executable search path.
    ${FLTK_DIR_SEARCH}

    # Look in standard UNIX install locations.
    /usr/local/lib/fltk
    /usr/lib/fltk
    /usr/local/include
    /usr/include
    /usr/local/fltk
    /usr/X11R6/include

    # Read from the CMakeSetup registry entries.  It is likely that
    # FLTK will have been recently built.
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild1]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild2]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild3]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild4]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild5]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild6]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild7]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild8]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild9]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild10]

    # Help the user find it if we cannot.
    DOC "The ${FLTK_DIR_STRING}"
    )
ENDIF(NOT FLTK_DIR)

# If FLTK was found, load the configuration file to get the rest of the
# settings.
IF(FLTK_DIR)

  # Check if FLTK was built using CMake
  IF(EXISTS ${FLTK_DIR}/FLTKConfig.cmake)
    SET(FLTK_BUILT_WITH_CMAKE 1)
  ENDIF(EXISTS ${FLTK_DIR}/FLTKConfig.cmake)

  IF(FLTK_BUILT_WITH_CMAKE)
    SET(FLTK_FOUND 1)
    INCLUDE(${FLTK_DIR}/FLTKConfig.cmake)

    # Fluid
    IF(FLUID_COMMAND) 
      SET(FLTK_FLUID_EXECUTABLE ${FLUID_COMMAND} CACHE FILEPATH "Fluid executable")
    ELSE(FLUID_COMMAND) 
      FIND_PROGRAM(FLTK_FLUID_EXECUTABLE fluid PATHS 
        ${FLTK_EXECUTABLE_DIRS}
        ${FLTK_EXECUTABLE_DIRS}/RelWithDebInfo
        ${FLTK_EXECUTABLE_DIRS}/Debug
        ${FLTK_EXECUTABLE_DIRS}/Release
        NO_SYSTEM_PATH)
    ENDIF(FLUID_COMMAND)
    # MARK_AS_ADVANCED(FLTK_FLUID_EXECUTABLE)

    SET(FLTK_INCLUDE_DIR ${FLTK_DIR})
    LINK_DIRECTORIES(${FLTK_LIBRARY_DIRS})

    SET(FLTK_BASE_LIBRARY fltk)
    SET(FLTK_GL_LIBRARY fltk_gl)
    SET(FLTK_FORMS_LIBRARY fltk_forms)
    SET(FLTK_IMAGES_LIBRARY fltk_images)

    # Add the extra libraries
    LOAD_CACHE(${FLTK_DIR}
      READ_WITH_PREFIX
      FL FLTK_USE_SYSTEM_JPEG
      FL FLTK_USE_SYSTEM_PNG
      FL FLTK_USE_SYSTEM_ZLIB
      )

    SET(FLTK_IMAGES_LIBS "")
    IF(FLFLTK_USE_SYSTEM_JPEG)
      SET(FLTK_IMAGES_LIBS ${FLTK_IMAGES_LIBS} fltk_jpeg)
    ENDIF(FLFLTK_USE_SYSTEM_JPEG)
    IF(FLFLTK_USE_SYSTEM_PNG)
      SET(FLTK_IMAGES_LIBS ${FLTK_IMAGES_LIBS} fltk_png)
    ENDIF(FLFLTK_USE_SYSTEM_PNG)
    IF(FLFLTK_USE_SYSTEM_ZLIB)
      SET(FLTK_IMAGES_LIBS ${FLTK_IMAGES_LIBS} fltk_zlib)
    ENDIF(FLFLTK_USE_SYSTEM_ZLIB)
    SET(FLTK_IMAGES_LIBS "${FLTK_IMAGES_LIBS}" CACHE INTERNAL
      "Extra libraries for fltk_images library.")

  ELSE(FLTK_BUILT_WITH_CMAKE)

    # if FLTK was not built using CMake
    # Find fluid executable.
    FIND_PROGRAM(FLTK_FLUID_EXECUTABLE fluid ${FLTK_INCLUDE_DIR}/fluid)

    # Use location of fluid to help find everything else.
    SET(FLTK_INCLUDE_SEARCH_PATH "")
    SET(FLTK_LIBRARY_SEARCH_PATH "")
    IF(FLTK_FLUID_EXECUTABLE)
      GET_FILENAME_COMPONENT(FLTK_BIN_DIR "${FLTK_FLUID_EXECUTABLE}" PATH)
      SET(FLTK_INCLUDE_SEARCH_PATH ${FLTK_INCLUDE_SEARCH_PATH}
        ${FLTK_BIN_DIR}/../include ${FLTK_BIN_DIR}/..)
      SET(FLTK_LIBRARY_SEARCH_PATH ${FLTK_LIBRARY_SEARCH_PATH}
        ${FLTK_BIN_DIR}/../lib)
      SET(FLTK_WRAP_UI 1)
    ENDIF(FLTK_FLUID_EXECUTABLE)

    SET(FLTK_INCLUDE_SEARCH_PATH ${FLTK_INCLUDE_SEARCH_PATH}
      /usr/local/include
      /usr/include
      /usr/local/fltk
      /usr/X11R6/include
      )

    FIND_PATH(FLTK_INCLUDE_DIR FL/Fl.h ${FLTK_INCLUDE_SEARCH_PATH})

    SET(FLTK_LIBRARY_SEARCH_PATH ${FLTK_LIBRARY_SEARCH_PATH}
      /usr/lib
      /usr/local/lib
      /usr/local/fltk/lib
      /usr/X11R6/lib
      ${FLTK_INCLUDE_DIR}/lib
      )

    FIND_LIBRARY(FLTK_BASE_LIBRARY NAMES fltk fltkd
      PATHS ${FLTK_LIBRARY_SEARCH_PATH})
    FIND_LIBRARY(FLTK_GL_LIBRARY NAMES fltkgl fltkgld fltk_gl
      PATHS ${FLTK_LIBRARY_SEARCH_PATH})
    FIND_LIBRARY(FLTK_FORMS_LIBRARY NAMES fltkforms fltkformsd fltk_forms
      PATHS ${FLTK_LIBRARY_SEARCH_PATH})
    FIND_LIBRARY(FLTK_IMAGES_LIBRARY NAMES fltkimages fltkimagesd fltk_images
      PATHS ${FLTK_LIBRARY_SEARCH_PATH})

    # Find the extra libraries needed for the fltk_images library.
    IF(UNIX)
      FIND_PROGRAM(FLTK_CONFIG_SCRIPT fltk-config PATHS ${FLTK_BIN_DIR})
      IF(FLTK_CONFIG_SCRIPT)
        EXEC_PROGRAM(${FLTK_CONFIG_SCRIPT} ARGS --use-images --ldflags
          OUTPUT_VARIABLE FLTK_IMAGES_LDFLAGS)
        SET(FLTK_LIBS_EXTRACT_REGEX ".*-lfltk_images (.*) -lfltk.*")
        IF("${FLTK_IMAGES_LDFLAGS}" MATCHES "${FLTK_LIBS_EXTRACT_REGEX}")
          STRING(REGEX REPLACE "${FLTK_LIBS_EXTRACT_REGEX}" "\\1"
            FLTK_IMAGES_LIBS "${FLTK_IMAGES_LDFLAGS}")
          STRING(REGEX REPLACE " +" ";" FLTK_IMAGES_LIBS "${FLTK_IMAGES_LIBS}")
          # The EXEC_PROGRAM will not be inherited into subdirectories from
          # the file that originally included this module.  Save the answer.
          SET(FLTK_IMAGES_LIBS "${FLTK_IMAGES_LIBS}" CACHE INTERNAL
            "Extra libraries for fltk_images library.")
        ENDIF("${FLTK_IMAGES_LDFLAGS}" MATCHES "${FLTK_LIBS_EXTRACT_REGEX}")
      ENDIF(FLTK_CONFIG_SCRIPT)
    ENDIF(UNIX)

  ENDIF(FLTK_BUILT_WITH_CMAKE)
ENDIF(FLTK_DIR)


SET(FLTK_FOUND 1)
FOREACH(var FLTK_FLUID_EXECUTABLE FLTK_INCLUDE_DIR
    FLTK_BASE_LIBRARY FLTK_GL_LIBRARY
    FLTK_FORMS_LIBRARY FLTK_IMAGES_LIBRARY)
  IF(NOT ${var})
    SET(FLTK_FOUND 0)
  ENDIF(NOT ${var})
ENDFOREACH(var)

IF(FLTK_FOUND)
  SET(FLTK_LIBRARIES ${FLTK_IMAGES_LIBRARY} ${FLTK_IMAGES_LIBS} ${FLTK_BASE_LIBRARY} ${FLTK_GL_LIBRARY}
    ${FLTK_FORMS_LIBRARY} )
  IF(APPLE)
    SET(FLTK_LIBRARIES ${FLTK_PLATFORM_DEPENDENT_LIBS} ${FLTK_LIBRARIES})
  ELSE(APPLE)
    SET(FLTK_LIBRARIES ${FLTK_LIBRARIES} ${FLTK_PLATFORM_DEPENDENT_LIBS})
  ENDIF(APPLE)

  # The following deprecated settings are for compatibility with CMake 1.4
  SET (HAS_FLTK ${FLTK_FOUND})
  SET (FLTK_INCLUDE_PATH ${FLTK_INCLUDE_DIR})
  SET (FLTK_FLUID_EXE ${FLTK_FLUID_EXECUTABLE})
  SET (FLTK_LIBRARY ${FLTK_LIBRARIES})
ENDIF(FLTK_FOUND)
