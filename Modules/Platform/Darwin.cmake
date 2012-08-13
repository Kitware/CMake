set(APPLE 1)

# Darwin versions:
#   6.x == Mac OSX 10.2 (Jaguar)
#   7.x == Mac OSX 10.3 (Panther)
#   8.x == Mac OSX 10.4 (Tiger)
#   9.x == Mac OSX 10.5 (Leopard)
#  10.x == Mac OSX 10.6 (Snow Leopard)
#  11.x == Mac OSX 10.7 (Lion)
#  12.x == Mac OSX 10.8 (Mountain Lion)
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+).*$" "\\1" DARWIN_MAJOR_VERSION "${CMAKE_SYSTEM_VERSION}")
string(REGEX REPLACE "^([0-9]+)\\.([0-9]+).*$" "\\2" DARWIN_MINOR_VERSION "${CMAKE_SYSTEM_VERSION}")

# Do not use the "-Wl,-search_paths_first" flag with the OSX 10.2 compiler.
# Done this way because it is too early to do a TRY_COMPILE.
if(NOT DEFINED HAVE_FLAG_SEARCH_PATHS_FIRST)
  set(HAVE_FLAG_SEARCH_PATHS_FIRST 0)
  if("${DARWIN_MAJOR_VERSION}" GREATER 6)
    set(HAVE_FLAG_SEARCH_PATHS_FIRST 1)
  endif()
endif()
# More desirable, but does not work:
  #include(CheckCXXCompilerFlag)
  #CHECK_CXX_COMPILER_FLAG("-Wl,-search_paths_first" HAVE_FLAG_SEARCH_PATHS_FIRST)

set(CMAKE_SHARED_LIBRARY_PREFIX "lib")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".dylib")
set(CMAKE_SHARED_MODULE_PREFIX "lib")
set(CMAKE_SHARED_MODULE_SUFFIX ".so")
set(CMAKE_MODULE_EXISTS 1)
set(CMAKE_DL_LIBS "")

set(CMAKE_C_OSX_COMPATIBILITY_VERSION_FLAG "-compatibility_version ")
set(CMAKE_C_OSX_CURRENT_VERSION_FLAG "-current_version ")
set(CMAKE_CXX_OSX_COMPATIBILITY_VERSION_FLAG "${CMAKE_C_OSX_COMPATIBILITY_VERSION_FLAG}")
set(CMAKE_CXX_OSX_CURRENT_VERSION_FLAG "${CMAKE_C_OSX_CURRENT_VERSION_FLAG}")

set(CMAKE_C_LINK_FLAGS "-Wl,-headerpad_max_install_names")
set(CMAKE_CXX_LINK_FLAGS "-Wl,-headerpad_max_install_names")

if(HAVE_FLAG_SEARCH_PATHS_FIRST)
  set(CMAKE_C_LINK_FLAGS "-Wl,-search_paths_first ${CMAKE_C_LINK_FLAGS}")
  set(CMAKE_CXX_LINK_FLAGS "-Wl,-search_paths_first ${CMAKE_CXX_LINK_FLAGS}")
endif()

set(CMAKE_PLATFORM_HAS_INSTALLNAME 1)
set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-dynamiclib -Wl,-headerpad_max_install_names")
set(CMAKE_SHARED_MODULE_CREATE_C_FLAGS "-bundle -Wl,-headerpad_max_install_names")
set(CMAKE_SHARED_MODULE_LOADER_C_FLAG "-Wl,-bundle_loader,")
set(CMAKE_SHARED_MODULE_LOADER_CXX_FLAG "-Wl,-bundle_loader,")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".dylib" ".so" ".a")

# hack: if a new cmake (which uses CMAKE_INSTALL_NAME_TOOL) runs on an old build tree
# (where install_name_tool was hardcoded) and where CMAKE_INSTALL_NAME_TOOL isn't in the cache
# and still cmake didn't fail in CMakeFindBinUtils.cmake (because it isn't rerun)
# hardcode CMAKE_INSTALL_NAME_TOOL here to install_name_tool, so it behaves as it did before, Alex
if(NOT DEFINED CMAKE_INSTALL_NAME_TOOL)
  find_program(CMAKE_INSTALL_NAME_TOOL install_name_tool)
  mark_as_advanced(CMAKE_INSTALL_NAME_TOOL)
endif()

# Set the assumed (Pre 10.5 or Default) location of the developer tools
set(OSX_DEVELOPER_ROOT "/Developer")

# Use the xcode-select tool if it's available (Xcode >= 3.0 installations)
find_program(CMAKE_XCODE_SELECT xcode-select)
mark_as_advanced(CMAKE_XCODE_SELECT)
if(CMAKE_XCODE_SELECT)
  execute_process(COMMAND ${CMAKE_XCODE_SELECT} "-print-path"
    OUTPUT_VARIABLE OSX_DEVELOPER_ROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

# Find installed SDKs
# Start with Xcode-4.3+ default SDKs directory
set(_CMAKE_OSX_SDKS_DIR
  "${OSX_DEVELOPER_ROOT}/Platforms/MacOSX.platform/Developer/SDKs")
file(GLOB _CMAKE_OSX_SDKS "${_CMAKE_OSX_SDKS_DIR}/*")

# If not present, try pre-4.3 SDKs directory
if(NOT _CMAKE_OSX_SDKS)
set(_CMAKE_OSX_SDKS_DIR "${OSX_DEVELOPER_ROOT}/SDKs")
  file(GLOB _CMAKE_OSX_SDKS "${_CMAKE_OSX_SDKS_DIR}/*")
endif()

execute_process(COMMAND sw_vers -productVersion
  OUTPUT_VARIABLE CURRENT_OSX_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)

#----------------------------------------------------------------------------
# _CURRENT_OSX_VERSION - as a two-component string: 10.5, 10.6, ...
#
string(REGEX REPLACE "^([0-9]+\\.[0-9]+).*$" "\\1"
  _CURRENT_OSX_VERSION "${CURRENT_OSX_VERSION}")

#----------------------------------------------------------------------------
# CMAKE_OSX_DEPLOYMENT_TARGET

# Set cache variable - end user may change this during ccmake or cmake-gui configure.
if(_CURRENT_OSX_VERSION VERSION_GREATER 10.3)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "$ENV{MACOSX_DEPLOYMENT_TARGET}" CACHE STRING
    "Minimum OS X version to target for deployment (at runtime); newer APIs weak linked. Set to empty string for default value.")
endif()

#----------------------------------------------------------------------------
# CMAKE_OSX_SYSROOT

# Environment variable set by the user overrides our default.
# Use the same environment variable that Xcode uses.
set(ENV_SDKROOT "$ENV{SDKROOT}")

# Set CMAKE_OSX_SYSROOT_DEFAULT based on _CURRENT_OSX_VERSION,
# accounting for the known specially named SDKs.
set(CMAKE_OSX_SYSROOT_DEFAULT
  "${_CMAKE_OSX_SDKS_DIR}/MacOSX${_CURRENT_OSX_VERSION}.sdk")

if(_CURRENT_OSX_VERSION STREQUAL "10.4")
  set(CMAKE_OSX_SYSROOT_DEFAULT
    "${_CMAKE_OSX_SDKS_DIR}/MacOSX10.4u.sdk")
endif()

if(_CURRENT_OSX_VERSION STREQUAL "10.3")
  set(CMAKE_OSX_SYSROOT_DEFAULT
    "${_CMAKE_OSX_SDKS_DIR}/MacOSX10.3.9.sdk")
endif()

# Use environment or default as initial cache value:
if(NOT ENV_SDKROOT STREQUAL "")
  set(CMAKE_OSX_SYSROOT_VALUE ${ENV_SDKROOT})
else()
  set(CMAKE_OSX_SYSROOT_VALUE ${CMAKE_OSX_SYSROOT_DEFAULT})
endif()

# Set cache variable - end user may change this during ccmake or cmake-gui configure.
set(CMAKE_OSX_SYSROOT ${CMAKE_OSX_SYSROOT_VALUE} CACHE PATH
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
endfunction()
#----------------------------------------------------------------------------

# Make sure the combination of SDK and Deployment Target are allowed
SanityCheckSDKAndDeployTarget("${CMAKE_OSX_SYSROOT}" "${CMAKE_OSX_DEPLOYMENT_TARGET}")

# set _CMAKE_OSX_MACHINE to uname -m
execute_process(COMMAND uname -m
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE _CMAKE_OSX_MACHINE)

# check for Power PC and change to ppc
if(_CMAKE_OSX_MACHINE MATCHES "Power")
  set(_CMAKE_OSX_MACHINE ppc)
endif()

# check for environment variable CMAKE_OSX_ARCHITECTURES
# if it is set.
if(NOT "$ENV{CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
  set(CMAKE_OSX_ARCHITECTURES_VALUE "$ENV{CMAKE_OSX_ARCHITECTURES}")
else()
  set(CMAKE_OSX_ARCHITECTURES_VALUE "")
endif()

# now put _CMAKE_OSX_MACHINE into the cache
set(CMAKE_OSX_ARCHITECTURES ${CMAKE_OSX_ARCHITECTURES_VALUE} CACHE STRING
  "Build architectures for OSX")


if("${CMAKE_BACKWARDS_COMPATIBILITY}" MATCHES "^1\\.[0-6]$")
  set(CMAKE_SHARED_MODULE_CREATE_C_FLAGS
    "${CMAKE_SHARED_MODULE_CREATE_C_FLAGS} -flat_namespace -undefined suppress")
endif()

if(NOT XCODE)
  # Enable shared library versioning.  This flag is not actually referenced
  # but the fact that the setting exists will cause the generators to support
  # soname computation.
  set(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "-install_name")
endif()

# Xcode does not support -isystem yet.
if(XCODE)
  set(CMAKE_INCLUDE_SYSTEM_FLAG_C)
  set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX)
endif()

if("${_CURRENT_OSX_VERSION}" VERSION_LESS "10.5")
  # Need to list dependent shared libraries on link line.  When building
  # with -isysroot (for universal binaries), the linker always looks for
  # dependent libraries under the sysroot.  Listing them on the link
  # line works around the problem.
  set(CMAKE_LINK_DEPENDENT_LIBRARY_FILES 1)
endif()

set(CMAKE_C_CREATE_SHARED_LIBRARY_FORBIDDEN_FLAGS -w)
set(CMAKE_CXX_CREATE_SHARED_LIBRARY_FORBIDDEN_FLAGS -w)
set(CMAKE_C_CREATE_SHARED_LIBRARY
  "<CMAKE_C_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> <LINK_FLAGS> -o <TARGET> <SONAME_FLAG> <TARGET_INSTALLNAME_DIR><TARGET_SONAME> <OBJECTS> <LINK_LIBRARIES>")
set(CMAKE_CXX_CREATE_SHARED_LIBRARY
  "<CMAKE_CXX_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS> <LINK_FLAGS> -o <TARGET> <SONAME_FLAG> <TARGET_INSTALLNAME_DIR><TARGET_SONAME> <OBJECTS> <LINK_LIBRARIES>")
set(CMAKE_Fortran_CREATE_SHARED_LIBRARY
  "<CMAKE_Fortran_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_Fortran_FLAGS> <LINK_FLAGS> -o <TARGET> <SONAME_FLAG> <TARGET_INSTALLNAME_DIR><TARGET_SONAME> <OBJECTS> <LINK_LIBRARIES>")

set(CMAKE_CXX_CREATE_SHARED_MODULE
      "<CMAKE_CXX_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS> <LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")

set(CMAKE_C_CREATE_SHARED_MODULE
      "<CMAKE_C_COMPILER>  <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_MODULE_CREATE_C_FLAGS> <LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")

set(CMAKE_Fortran_CREATE_SHARED_MODULE
      "<CMAKE_Fortran_COMPILER>  <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_MODULE_CREATE_Fortran_FLAGS> <LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")

set(CMAKE_C_CREATE_MACOSX_FRAMEWORK
      "<CMAKE_C_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> <LINK_FLAGS> -o <TARGET> <SONAME_FLAG> <TARGET_INSTALLNAME_DIR><TARGET_SONAME> <OBJECTS> <LINK_LIBRARIES>")
set(CMAKE_CXX_CREATE_MACOSX_FRAMEWORK
      "<CMAKE_CXX_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS> <LINK_FLAGS> -o <TARGET> <SONAME_FLAG> <TARGET_INSTALLNAME_DIR><TARGET_SONAME> <OBJECTS> <LINK_LIBRARIES>")


# default to searching for frameworks first
if(NOT DEFINED CMAKE_FIND_FRAMEWORK)
  set(CMAKE_FIND_FRAMEWORK FIRST)
endif()
# set up the default search directories for frameworks
set(CMAKE_SYSTEM_FRAMEWORK_PATH
  ~/Library/Frameworks
  /Library/Frameworks
  /Network/Library/Frameworks
  /System/Library/Frameworks)

# default to searching for application bundles first
if(NOT DEFINED CMAKE_FIND_APPBUNDLE)
  set(CMAKE_FIND_APPBUNDLE FIRST)
endif()
# set up the default search directories for application bundles
set(_apps_paths)
foreach(_path
  "~/Applications"
  "/Applications"
  "${OSX_DEVELOPER_ROOT}/../Applications" # Xcode 4.3+
  "${OSX_DEVELOPER_ROOT}/Applications"    # pre-4.3
  )
  get_filename_component(_apps "${_path}" ABSOLUTE)
  if(EXISTS "${_apps}")
    list(APPEND _apps_paths "${_apps}")
  endif()
endforeach()
list(REMOVE_DUPLICATES _apps_paths)
set(CMAKE_SYSTEM_APPBUNDLE_PATH
  ${_apps_paths})
unset(_apps_paths)

include(Platform/UnixPaths)
list(APPEND CMAKE_SYSTEM_PREFIX_PATH
  /sw        # Fink
  /opt/local # MacPorts
  )
