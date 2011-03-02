SET(CMAKE_PLATFORM_REQUIRED_RUNTIME_PATH /usr/lib)

SET(CMAKE_SHARED_LIBRARY_SUFFIX ".sl")          # .so
SET(CMAKE_DL_LIBS "dld")
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".sl" ".so" ".a")
SET(CMAKE_EXTRA_SHARED_LIBRARY_SUFFIXES ".so")

# The HP linker needs to find transitive shared library dependencies
# in the -L path.  Therefore the runtime path must be added to the
# link line with -L flags.
SET(CMAKE_SHARED_LIBRARY_LINK_C_WITH_RUNTIME_PATH 1)
SET(CMAKE_LINK_DEPENDENT_LIBRARY_DIRS 1)

# Shared libraries with no builtin soname may not be linked safely by
# specifying the file path.
SET(CMAKE_PLATFORM_USES_PATH_WHEN_NO_SONAME 1)

# set flags for gcc support
INCLUDE(Platform/UnixPaths)

# Look in both 32-bit and 64-bit implict link directories, but tell
# CMake not to pass the paths to the linker.  The linker will find the
# library for the proper architecture.  In the future we should detect
# which path will be used by the linker.  Since the pointer type size
# CMAKE_SIZEOF_VOID_P is not set until after this file executes, we
# would need to append to CMAKE_SYSTEM_LIBRARY_PATH at a later point
# (after CMakeTest(LANG)Compiler.cmake runs for at least one language).
LIST(APPEND CMAKE_SYSTEM_LIBRARY_PATH /usr/lib/hpux32)
LIST(APPEND CMAKE_SYSTEM_LIBRARY_PATH /usr/lib/hpux64)
LIST(APPEND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
  /usr/lib/hpux32 /usr/lib/hpux64)

# Initialize C and CXX link type selection flags.  These flags are
# used when building a shared library, shared module, or executable
# that links to other libraries to select whether to use the static or
# shared versions of the libraries.  Note that C modules and shared
# libs are built using ld directly so we leave off the "-Wl," portion.
FOREACH(type SHARED_LIBRARY SHARED_MODULE)
  SET(CMAKE_${type}_LINK_STATIC_C_FLAGS "-a archive")
  SET(CMAKE_${type}_LINK_DYNAMIC_C_FLAGS "-a default")
ENDFOREACH(type)
FOREACH(type EXE)
  SET(CMAKE_${type}_LINK_STATIC_C_FLAGS "-Wl,-a,archive")
  SET(CMAKE_${type}_LINK_DYNAMIC_C_FLAGS "-Wl,-a,default")
ENDFOREACH(type)
FOREACH(type SHARED_LIBRARY SHARED_MODULE EXE)
  SET(CMAKE_${type}_LINK_STATIC_CXX_FLAGS "-Wl,-a,archive")
  SET(CMAKE_${type}_LINK_DYNAMIC_CXX_FLAGS "-Wl,-a,default")
ENDFOREACH(type)

