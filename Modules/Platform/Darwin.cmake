SET(APPLE 1)

# Darwin versions:
#   6.x == Mac OSX 10.2 (Jaguar)
#   7.x == Mac OSX 10.3 (Panther)
#   8.x == Mac OSX 10.4 (Tiger)
#   9.x == Mac OSX 10.5 (Leopard)
#  10.x == Mac OSX 10.6 (Snow Leopard)
STRING(REGEX REPLACE "^([0-9]+)\\.([0-9]+).*$" "\\1" DARWIN_MAJOR_VERSION "${CMAKE_SYSTEM_VERSION}")
STRING(REGEX REPLACE "^([0-9]+)\\.([0-9]+).*$" "\\2" DARWIN_MINOR_VERSION "${CMAKE_SYSTEM_VERSION}")

# Do not use the "-Wl,-search_paths_first" flag with the OSX 10.2 compiler.
# Done this way because it is too early to do a TRY_COMPILE.
IF(NOT DEFINED HAVE_FLAG_SEARCH_PATHS_FIRST)
  SET(HAVE_FLAG_SEARCH_PATHS_FIRST 0)
  IF("${DARWIN_MAJOR_VERSION}" GREATER 6)
    SET(HAVE_FLAG_SEARCH_PATHS_FIRST 1)
  ENDIF("${DARWIN_MAJOR_VERSION}" GREATER 6)
ENDIF(NOT DEFINED HAVE_FLAG_SEARCH_PATHS_FIRST)
# More desirable, but does not work:
  #INCLUDE(CheckCXXCompilerFlag)
  #CHECK_CXX_COMPILER_FLAG("-Wl,-search_paths_first" HAVE_FLAG_SEARCH_PATHS_FIRST)

SET(CMAKE_SHARED_LIBRARY_PREFIX "lib")
SET(CMAKE_SHARED_LIBRARY_SUFFIX ".dylib")
SET(CMAKE_SHARED_MODULE_PREFIX "lib")
SET(CMAKE_SHARED_MODULE_SUFFIX ".so")
SET(CMAKE_MODULE_EXISTS 1)
SET(CMAKE_DL_LIBS "")

SET(CMAKE_C_OSX_COMPATIBILITY_VERSION_FLAG "-compatibility_version ")
SET(CMAKE_C_OSX_CURRENT_VERSION_FLAG "-current_version ")
SET(CMAKE_CXX_OSX_COMPATIBILITY_VERSION_FLAG "${CMAKE_C_OSX_COMPATIBILITY_VERSION_FLAG}")
SET(CMAKE_CXX_OSX_CURRENT_VERSION_FLAG "${CMAKE_C_OSX_CURRENT_VERSION_FLAG}")

SET(CMAKE_C_LINK_FLAGS "-Wl,-headerpad_max_install_names")
SET(CMAKE_CXX_LINK_FLAGS "-Wl,-headerpad_max_install_names")

IF(HAVE_FLAG_SEARCH_PATHS_FIRST)
  SET(CMAKE_C_LINK_FLAGS "-Wl,-search_paths_first ${CMAKE_C_LINK_FLAGS}")
  SET(CMAKE_CXX_LINK_FLAGS "-Wl,-search_paths_first ${CMAKE_CXX_LINK_FLAGS}")
ENDIF(HAVE_FLAG_SEARCH_PATHS_FIRST)

SET(CMAKE_PLATFORM_HAS_INSTALLNAME 1)
SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-dynamiclib -Wl,-headerpad_max_install_names")
SET(CMAKE_SHARED_MODULE_CREATE_C_FLAGS "-bundle -Wl,-headerpad_max_install_names")
SET(CMAKE_SHARED_MODULE_LOADER_C_FLAG "-Wl,-bundle_loader,")
SET(CMAKE_SHARED_MODULE_LOADER_CXX_FLAG "-Wl,-bundle_loader,")
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dylib" ".so" ".a")

# hack: if a new cmake (which uses CMAKE_INSTALL_NAME_TOOL) runs on an old build tree
# (where install_name_tool was hardcoded) and where CMAKE_INSTALL_NAME_TOOL isn't in the cache
# and still cmake didn't fail in CMakeFindBinUtils.cmake (because it isn't rerun)
# hardcode CMAKE_INSTALL_NAME_TOOL here to install_name_tool, so it behaves as it did before, Alex
IF(NOT DEFINED CMAKE_INSTALL_NAME_TOOL)
  FIND_PROGRAM(CMAKE_INSTALL_NAME_TOOL install_name_tool)
ENDIF(NOT DEFINED CMAKE_INSTALL_NAME_TOOL)

# Set the assumed (Pre 10.5 or Default) location of the developer tools
SET(OSX_DEVELOPER_ROOT "/Developer")

# Find installed SDKs
FILE(GLOB _CMAKE_OSX_SDKS "${OSX_DEVELOPER_ROOT}/SDKs/*")

# If nothing is found there, then try locating the dev tools based on the xcode-select tool
# (available in Xcode >= 3.0 installations)
IF(NOT _CMAKE_OSX_SDKS)
  FIND_PROGRAM(CMAKE_XCODE_SELECT xcode-select)
  IF(CMAKE_XCODE_SELECT)
    EXECUTE_PROCESS(COMMAND ${CMAKE_XCODE_SELECT} "-print-path"
      OUTPUT_VARIABLE OSX_DEVELOPER_ROOT
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    FILE(GLOB _CMAKE_OSX_SDKS "${OSX_DEVELOPER_ROOT}/SDKs/*")
  ENDIF(CMAKE_XCODE_SELECT)
ENDIF(NOT _CMAKE_OSX_SDKS)

EXECUTE_PROCESS(COMMAND sw_vers -productVersion
  OUTPUT_VARIABLE CURRENT_OSX_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)

#----------------------------------------------------------------------------
# _CURRENT_OSX_VERSION - as a two-component string: 10.5, 10.6, ...
#
STRING(REGEX REPLACE "^([0-9]+\\.[0-9]+).*$" "\\1"
  _CURRENT_OSX_VERSION "${CURRENT_OSX_VERSION}")

#----------------------------------------------------------------------------
# CMAKE_OSX_DEPLOYMENT_TARGET

# Set cache variable - end user may change this during ccmake or cmake-gui configure.
IF(_CURRENT_OSX_VERSION VERSION_GREATER 10.3)
  SET(CMAKE_OSX_DEPLOYMENT_TARGET "$ENV{MACOSX_DEPLOYMENT_TARGET}" CACHE STRING
    "Minimum OS X version to target for deployment (at runtime); newer APIs weak linked. Set to empty string for default value.")
ENDIF(_CURRENT_OSX_VERSION VERSION_GREATER 10.3)

#----------------------------------------------------------------------------
# CMAKE_OSX_SYSROOT

# Environment variable set by the user overrides our default.
# Use the same environment variable that Xcode uses.
SET(ENV_SDKROOT "$ENV{SDKROOT}")

# Set CMAKE_OSX_SYSROOT_DEFAULT based on _CURRENT_OSX_VERSION,
# accounting for the known specially named SDKs.
SET(CMAKE_OSX_SYSROOT_DEFAULT
  "${OSX_DEVELOPER_ROOT}/SDKs/MacOSX${_CURRENT_OSX_VERSION}.sdk")

IF(_CURRENT_OSX_VERSION STREQUAL "10.4")
  SET(CMAKE_OSX_SYSROOT_DEFAULT
    "${OSX_DEVELOPER_ROOT}/SDKs/MacOSX10.4u.sdk")
ENDIF(_CURRENT_OSX_VERSION STREQUAL "10.4")

IF(_CURRENT_OSX_VERSION STREQUAL "10.3")
  SET(CMAKE_OSX_SYSROOT_DEFAULT
    "${OSX_DEVELOPER_ROOT}/SDKs/MacOSX10.3.9.sdk")
ENDIF(_CURRENT_OSX_VERSION STREQUAL "10.3")

# Use environment or default as initial cache value:
IF(NOT ENV_SDKROOT STREQUAL "")
  SET(CMAKE_OSX_SYSROOT_VALUE ${ENV_SDKROOT})
ELSE(NOT ENV_SDKROOT STREQUAL "")
  SET(CMAKE_OSX_SYSROOT_VALUE ${CMAKE_OSX_SYSROOT_DEFAULT})
ENDIF(NOT ENV_SDKROOT STREQUAL "")

# Set cache variable - end user may change this during ccmake or cmake-gui configure.
SET(CMAKE_OSX_SYSROOT ${CMAKE_OSX_SYSROOT_VALUE} CACHE PATH
  "The product will be built against the headers and libraries located inside the indicated SDK.")

#----------------------------------------------------------------------------
function(SanityCheckSDKAndDeployTarget _sdk_path _deploy)
  if(_deploy STREQUAL "")
    return()
  endif()

  if(_sdk_path STREQUAL "")
    message(FATAL_ERROR "CMAKE_OSX_DEPLOYMENT_TARGET='${_deploy}' but CMAKE_OSX_SYSROOT is empty... - either set CMAKE_OSX_SYSROOT to a valid SDK or set CMAKE_OSX_DEPLOYMENT_TARGET to empty")
  endif()

  string(REGEX REPLACE "(.*MacOSX*)(....)(.*\\.sdk)" "\\2" SDK "${_sdk_path}")
  if(_deploy GREATER "${SDK}")
    message(FATAL_ERROR "CMAKE_OSX_DEPLOYMENT_TARGET (${_deploy}) is greater than CMAKE_OSX_SYSROOT SDK (${_sdk_path}). Please set CMAKE_OSX_DEPLOYMENT_TARGET to ${SDK} or lower")
  endif()
endfunction(SanityCheckSDKAndDeployTarget)
#----------------------------------------------------------------------------

# Make sure the combination of SDK and Deployment Target are allowed
SanityCheckSDKAndDeployTarget("${CMAKE_OSX_SYSROOT}" "${CMAKE_OSX_DEPLOYMENT_TARGET}")

# set _CMAKE_OSX_MACHINE to uname -m
EXECUTE_PROCESS(COMMAND uname -m
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE _CMAKE_OSX_MACHINE)

# check for Power PC and change to ppc
IF(_CMAKE_OSX_MACHINE MATCHES "Power")
  SET(_CMAKE_OSX_MACHINE ppc)
ENDIF(_CMAKE_OSX_MACHINE MATCHES "Power")

# check for environment variable CMAKE_OSX_ARCHITECTURES
# if it is set.
IF(NOT "$ENV{CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
  SET(CMAKE_OSX_ARCHITECTURES_VALUE "$ENV{CMAKE_OSX_ARCHITECTURES}")
ELSE(NOT "$ENV{CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
  SET(CMAKE_OSX_ARCHITECTURES_VALUE "")
ENDIF(NOT "$ENV{CMAKE_OSX_ARCHITECTURES}" STREQUAL "")

# now put _CMAKE_OSX_MACHINE into the cache
SET(CMAKE_OSX_ARCHITECTURES ${CMAKE_OSX_ARCHITECTURES_VALUE} CACHE STRING
  "Build architectures for OSX")


IF("${CMAKE_BACKWARDS_COMPATIBILITY}" MATCHES "^1\\.[0-6]$")
  SET(CMAKE_SHARED_MODULE_CREATE_C_FLAGS
    "${CMAKE_SHARED_MODULE_CREATE_C_FLAGS} -flat_namespace -undefined suppress")
ENDIF("${CMAKE_BACKWARDS_COMPATIBILITY}" MATCHES "^1\\.[0-6]$")

IF(NOT XCODE)
  # Enable shared library versioning.  This flag is not actually referenced
  # but the fact that the setting exists will cause the generators to support
  # soname computation.
  SET(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "-install_name")
ENDIF(NOT XCODE)

# Xcode does not support -isystem yet.
IF(XCODE)
  SET(CMAKE_INCLUDE_SYSTEM_FLAG_C)
  SET(CMAKE_INCLUDE_SYSTEM_FLAG_CXX)
ENDIF(XCODE)

IF("${_CURRENT_OSX_VERSION}" VERSION_LESS "10.5")
  # Need to list dependent shared libraries on link line.  When building
  # with -isysroot (for universal binaries), the linker always looks for
  # dependent libraries under the sysroot.  Listing them on the link
  # line works around the problem.
  SET(CMAKE_LINK_DEPENDENT_LIBRARY_FILES 1)
ENDIF()

SET(CMAKE_C_CREATE_SHARED_LIBRARY_FORBIDDEN_FLAGS -w)
SET(CMAKE_CXX_CREATE_SHARED_LIBRARY_FORBIDDEN_FLAGS -w)
SET(CMAKE_C_CREATE_SHARED_LIBRARY
  "<CMAKE_C_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> <LINK_FLAGS> -o <TARGET> -install_name <TARGET_INSTALLNAME_DIR><TARGET_SONAME> <OBJECTS> <LINK_LIBRARIES>")
SET(CMAKE_CXX_CREATE_SHARED_LIBRARY
  "<CMAKE_CXX_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS> <LINK_FLAGS> -o <TARGET> -install_name <TARGET_INSTALLNAME_DIR><TARGET_SONAME> <OBJECTS> <LINK_LIBRARIES>")
SET(CMAKE_Fortran_CREATE_SHARED_LIBRARY
  "<CMAKE_Fortran_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_Fortran_FLAGS> <LINK_FLAGS> -o <TARGET> -install_name <TARGET_INSTALLNAME_DIR><TARGET_SONAME> <OBJECTS> <LINK_LIBRARIES>")

SET(CMAKE_CXX_CREATE_SHARED_MODULE
      "<CMAKE_CXX_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS> <LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")

SET(CMAKE_C_CREATE_SHARED_MODULE
      "<CMAKE_C_COMPILER>  <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_MODULE_CREATE_C_FLAGS> <LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")

SET(CMAKE_Fortran_CREATE_SHARED_MODULE
      "<CMAKE_Fortran_COMPILER>  <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_MODULE_CREATE_Fortran_FLAGS> <LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")

SET(CMAKE_C_CREATE_MACOSX_FRAMEWORK
      "<CMAKE_C_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> <LINK_FLAGS> -o <TARGET> -install_name <TARGET_INSTALLNAME_DIR><TARGET_SONAME> <OBJECTS> <LINK_LIBRARIES>")
SET(CMAKE_CXX_CREATE_MACOSX_FRAMEWORK
      "<CMAKE_CXX_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS> <LINK_FLAGS> -o <TARGET> -install_name <TARGET_INSTALLNAME_DIR><TARGET_SONAME> <OBJECTS> <LINK_LIBRARIES>")


 
# default to searching for frameworks first
SET(CMAKE_FIND_FRAMEWORK FIRST)
# set up the default search directories for frameworks
SET(CMAKE_SYSTEM_FRAMEWORK_PATH
  ~/Library/Frameworks
  /Library/Frameworks
  /Network/Library/Frameworks
  /System/Library/Frameworks)

# default to searching for application bundles first
SET(CMAKE_FIND_APPBUNDLE FIRST)
# set up the default search directories for application bundles
SET(CMAKE_SYSTEM_APPBUNDLE_PATH
  ~/Applications
  /Applications
  /Developer/Applications)

INCLUDE(Platform/UnixPaths)
LIST(APPEND CMAKE_SYSTEM_PREFIX_PATH
  /sw        # Fink
  /opt/local # MacPorts
  )
