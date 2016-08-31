# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

set(CMAKE_CUDA_OUTPUT_EXTENSION .o)
set(CMAKE_INCLUDE_FLAG_CUDA "-I")

# Load compiler-specific information.
if(CMAKE_CUDA_COMPILER_ID)
  include(Compiler/${CMAKE_CUDA_COMPILER_ID}-CUDA OPTIONAL)
endif()

# load the system- and compiler specific files
if(CMAKE_CUDA_COMPILER_ID)
  # load a hardware specific file, mostly useful for embedded compilers
  if(CMAKE_SYSTEM_PROCESSOR)
    include(Platform/${CMAKE_SYSTEM_NAME}-${CMAKE_CUDA_COMPILER_ID}-CUDA-${CMAKE_SYSTEM_PROCESSOR} OPTIONAL)
  endif()
  include(Platform/${CMAKE_SYSTEM_NAME}-${CMAKE_CUDA_COMPILER_ID}-CUDA OPTIONAL)
endif()

# for most systems a module is the same as a shared library
# so unless the variable CMAKE_MODULE_EXISTS is set just
# copy the values from the LIBRARY variables
if(NOT CMAKE_MODULE_EXISTS)
  set(CMAKE_SHARED_MODULE_CUDA_FLAGS ${CMAKE_SHARED_LIBRARY_CUDA_FLAGS})
  set(CMAKE_SHARED_MODULE_CREATE_CUDA_FLAGS ${CMAKE_SHARED_LIBRARY_CREATE_CUDA_FLAGS})
endif()

# add the flags to the cache based
# on the initial values computed in the platform/*.cmake files
# use _INIT variables so that this only happens the first time
# and you can set these flags in the cmake cache
set(CMAKE_CUDA_FLAGS_INIT "$ENV{CUDAFLAGS} ${CMAKE_CUDA_FLAGS_INIT}")

foreach(c "" _DEBUG _RELEASE _MINSIZEREL _RELWITHDEBINFO)
  string(STRIP "${CMAKE_CUDA_FLAGS${c}_INIT}" CMAKE_CUDA_FLAGS${c}_INIT)
endforeach()

set (CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS_INIT}" CACHE STRING
     "Flags used by the compiler during all build types.")

if(NOT CMAKE_NOT_USING_CONFIG_FLAGS)
  set (CMAKE_CUDA_FLAGS_DEBUG "${CMAKE_CUDA_FLAGS_DEBUG_INIT}" CACHE STRING
     "Flags used by the compiler during debug builds.")
  set (CMAKE_CUDA_FLAGS_MINSIZEREL "${CMAKE_CUDA_FLAGS_MINSIZEREL_INIT}" CACHE STRING
     "Flags used by the compiler during release builds for minimum size.")
  set (CMAKE_CUDA_FLAGS_RELEASE "${CMAKE_CUDA_FLAGS_RELEASE_INIT}" CACHE STRING
     "Flags used by the compiler during release builds.")
  set (CMAKE_CUDA_FLAGS_RELWITHDEBINFO "${CMAKE_CUDA_FLAGS_RELWITHDEBINFO_INIT}" CACHE STRING
     "Flags used by the compiler during release builds with debug info.")

endif()

include(CMakeCommonLanguageInclude)

# now define the following rules:
# CMAKE_CUDA_CREATE_SHARED_LIBRARY
# CMAKE_CUDA_CREATE_SHARED_MODULE
# CMAKE_CUDA_COMPILE_OBJECT
# CMAKE_CUDA_LINK_EXECUTABLE

# create a shared library
if(NOT CMAKE_CUDA_CREATE_SHARED_LIBRARY)
  set(CMAKE_CUDA_CREATE_SHARED_LIBRARY
      "<CMAKE_CUDA_COMPILER> <CMAKE_SHARED_LIBRARY_CUDA_FLAGS> <LANGUAGE_COMPILE_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_CUDA_FLAGS> <SONAME_FLAG><TARGET_SONAME> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
endif()

# create a shared module copy the shared library rule by default
if(NOT CMAKE_CUDA_CREATE_SHARED_MODULE)
  set(CMAKE_CUDA_CREATE_SHARED_MODULE ${CMAKE_CUDA_CREATE_SHARED_LIBRARY})
endif()

# Create a static archive incrementally for large object file counts.
if(NOT DEFINED CMAKE_CUDA_ARCHIVE_CREATE)
  set(CMAKE_CUDA_ARCHIVE_CREATE "<CMAKE_AR> qc <TARGET> <LINK_FLAGS> <OBJECTS>")
endif()
if(NOT DEFINED CMAKE_CUDA_ARCHIVE_APPEND)
  set(CMAKE_CUDA_ARCHIVE_APPEND "<CMAKE_AR> q  <TARGET> <LINK_FLAGS> <OBJECTS>")
endif()
if(NOT DEFINED CMAKE_CUDA_ARCHIVE_FINISH)
  set(CMAKE_CUDA_ARCHIVE_FINISH "<CMAKE_RANLIB> <TARGET>")
endif()


# compile a cu file into an object file
if(NOT CMAKE_CUDA_COMPILE_OBJECT)
  set(CMAKE_CUDA_COMPILE_OBJECT
    "<CMAKE_CUDA_COMPILER>  <DEFINES> <INCLUDES> <FLAGS> -x cu -c <SOURCE> -o <OBJECT>")

  #The Ninja generator uses the make file dependency files to determine what
  #files need to be recompiled. Unfortunately, nvcc doesn't support building
  #a source file and generating the dependencies of said file in a single
  #invocation. Instead we have to state that you need to chain two commands.
  #
  #The makefile generators uses the custom CMake dependency scanner, and thus
  #it is exempt from this logic.
  if(CMAKE_GENERATOR STREQUAL "Ninja")
    list(APPEND CMAKE_CUDA_COMPILE_OBJECT
      "<CMAKE_CUDA_COMPILER>  <DEFINES> <INCLUDES> <FLAGS> -x cu -M <SOURCE> -MT <OBJECT> -o $DEP_FILE")
  endif()

endif()

# compile a cu file into an executable
if(NOT CMAKE_CUDA_LINK_EXECUTABLE)
  set(CMAKE_CUDA_LINK_EXECUTABLE
    "<CMAKE_CUDA_COMPILER>  <FLAGS> <CMAKE_CUDA_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")
endif()


mark_as_advanced(
CMAKE_CUDA_FLAGS
CMAKE_CUDA_FLAGS_RELEASE
CMAKE_CUDA_FLAGS_RELWITHDEBINFO
CMAKE_CUDA_FLAGS_MINSIZEREL
CMAKE_CUDA_FLAGS_DEBUG)

set(CMAKE_CUDA_INFORMATION_LOADED 1)
