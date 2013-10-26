set(CMAKE_C_CREATE_SHARED_LIBRARY_FORBIDDEN_FLAGS "" )
set(CMAKE_CXX_CREATE_SHARED_LIBRARY_FORBIDDEN_FLAGS "")

# Setup for Leopard Compatibility
exec_program(sw_vers ARGS -productVersion OUTPUT_VARIABLE _OSX_VERSION)
# message (STATUS "_OSX_VERSION: ${_OSX_VERSION}")
if ( _OSX_VERSION MATCHES "^10.4" )
  #if(CMAKE_COMPILER_IS_GNUCC)
    set (CMAKE_C_FLAGS_INIT "")
    set (CMAKE_C_FLAGS_DEBUG_INIT "-gdwarf-2")
    set (CMAKE_C_FLAGS_MINSIZEREL_INIT "-Os -DNDEBUG")
    set (CMAKE_C_FLAGS_RELEASE_INIT "-O3 -DNDEBUG")
    set (CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "-O2 -gdwarf-2")
    set (CMAKE_C_CREATE_PREPROCESSED_SOURCE "<CMAKE_C_COMPILER> <FLAGS> -E <SOURCE> > <PREPROCESSED_SOURCE>")
    set (CMAKE_C_CREATE_ASSEMBLY_SOURCE "<CMAKE_C_COMPILER> <FLAGS> -S <SOURCE> -o <ASSEMBLY_SOURCE>")
 # endif()

#  if(CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_CXX_FLAGS_INIT "")
    set (CMAKE_CXX_FLAGS_DEBUG_INIT "-gdwarf-2")
    set (CMAKE_CXX_FLAGS_MINSIZEREL_INIT "-Os -DNDEBUG")
    set (CMAKE_CXX_FLAGS_RELEASE_INIT "-O3 -DNDEBUG")
    set (CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-O2 -gdwarf-2")
    set (CMAKE_CXX_CREATE_PREPROCESSED_SOURCE "<CMAKE_CXX_COMPILER> <FLAGS> -E <SOURCE> > <PREPROCESSED_SOURCE>")
    set (CMAKE_CXX_CREATE_ASSEMBLY_SOURCE "<CMAKE_CXX_COMPILER> <FLAGS> -S <SOURCE> -o <ASSEMBLY_SOURCE>")
#  endif()
endif ()


set(CMAKE_SHARED_LIBRARY_PREFIX "lib")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".dylib")
set(CMAKE_SHARED_MODULE_PREFIX "lib")
set(CMAKE_SHARED_MODULE_SUFFIX ".so")
set(CMAKE_MODULE_EXISTS 1)
set(CMAKE_DL_LIBS "")
set(CMAKE_C_LINK_FLAGS "-Wl,-headerpad_max_install_names")
set(CMAKE_CXX_LINK_FLAGS "-Wl,-headerpad_max_install_names")
set(CMAKE_PLATFORM_HAS_INSTALLNAME 1)
set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-dynamiclib -Wl,-headerpad_max_install_names")
set(CMAKE_SHARED_MODULE_CREATE_C_FLAGS "-bundle -Wl,-headerpad_max_install_names")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".dylib" ".so" ".a")


# setup for universal binaries if sysroot exists
if(EXISTS /Developer/SDKs/MacOSX10.4u.sdk)
  # set the sysroot to be used if CMAKE_OSX_ARCHITECTURES
  # has more than one value
  set(CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX10.4u.sdk CACHE STRING
    "isysroot used for universal binary support")
  # set _CMAKE_OSX_MACHINE to umame -m
  exec_program(uname ARGS -m OUTPUT_VARIABLE _CMAKE_OSX_MACHINE)

  # check for environment variable CMAKE_OSX_ARCHITECTURES
  # if it is set.
  if(NOT "$ENV{CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
    set(_CMAKE_OSX_MACHINE "$ENV{CMAKE_OSX_ARCHITECTURES}")
  endif()
  # now put _CMAKE_OSX_MACHINE into the cache
  set(CMAKE_OSX_ARCHITECTURES ${_CMAKE_OSX_MACHINE}
    CACHE STRING "Build architectures for OSX")
endif()

if(NOT XCODE)
  # Enable shared library versioning.  This flag is not actually referenced
  # but the fact that the setting exists will cause the generators to support
  # soname computation.
  set(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "-install_name")
  set(CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG "-install_name")
  set(CMAKE_SHARED_LIBRARY_SONAME_Fortran_FLAG "-install_name")
endif()

# Xcode does not support -isystem yet.
if(XCODE)
  set(CMAKE_INCLUDE_SYSTEM_FLAG_C)
  set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX)
endif()

set(CMAKE_MacOSX_Content_COMPILE_OBJECT "\"${CMAKE_COMMAND}\" -E copy_if_different <SOURCE> <OBJECT>")

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


#  We can use $ENV{INTEL_LICENSE_FILE} to try and get at the installation location for ICC.
# We also need to consider to use cce (which is the 64bit compiler) and not JUST the 32bit compiler.
# I have no idea what the best way to do that would be.


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
set(CMAKE_SYSTEM_APPBUNDLE_PATH
  ~/Applications
  /Applications
  /Developer/Applications)

include(Platform/UnixPaths)
set(CMAKE_SYSTEM_INCLUDE_PATH ${CMAKE_SYSTEM_INCLUDE_PATH} /sw/include)
set(CMAKE_SYSTEM_LIBRARY_PATH ${CMAKE_SYSTEM_LIBRARY_PATH} /sw/lib)
