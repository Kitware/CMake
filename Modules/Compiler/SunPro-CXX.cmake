set(CMAKE_CXX_VERBOSE_FLAG "-v")

set(CMAKE_CXX_COMPILE_OPTIONS_PIC -KPIC)
set(CMAKE_CXX_COMPILE_OPTIONS_PIE -KPIE)
set(CMAKE_SHARED_LIBRARY_CXX_FLAGS "-KPIC")
set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "-G")
set(CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG "-R")
set(CMAKE_SHARED_LIBRARY_RUNTIME_CXX_FLAG_SEP ":")
set(CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG "-h")

set(CMAKE_CXX_FLAGS_INIT "")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g")
set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "-xO2 -xspace -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-xO3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-g -xO2 -DNDEBUG")

# Initialize C link type selection flags.  These flags are used when
# building a shared library, shared module, or executable that links
# to other libraries to select whether to use the static or shared
# versions of the libraries.
foreach(type SHARED_LIBRARY SHARED_MODULE EXE)
  set(CMAKE_${type}_LINK_STATIC_CXX_FLAGS "-Bstatic")
  set(CMAKE_${type}_LINK_DYNAMIC_CXX_FLAGS "-Bdynamic")
endforeach()

set(CMAKE_CXX_CREATE_PREPROCESSED_SOURCE "<CMAKE_CXX_COMPILER> <DEFINES> <FLAGS> -E <SOURCE> > <PREPROCESSED_SOURCE>")
set(CMAKE_CXX_CREATE_ASSEMBLY_SOURCE "<CMAKE_CXX_COMPILER> <FLAGS> -S <SOURCE> -o <ASSEMBLY_SOURCE>")

# Create archives with "CC -xar" in case user adds "-instances=extern"
# so that template instantiations are available to archive members.
set(CMAKE_CXX_CREATE_STATIC_LIBRARY
  "<CMAKE_CXX_COMPILER> -xar -o <TARGET> <OBJECTS> "
  "<CMAKE_RANLIB> <TARGET> ")
