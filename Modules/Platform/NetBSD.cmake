if(EXISTS /usr/include/dlfcn.h)
  set(CMAKE_DL_LIBS "")
  set(CMAKE_SHARED_LIBRARY_C_FLAGS "-fPIC")            # -pic
  set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-shared")       # -shared
  set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")         # +s, flag for exe link to use shared lib
  set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,-rpath,")       # -rpath
  set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP ":")   # : or empty
  set(CMAKE_SHARED_LIBRARY_RPATH_LINK_C_FLAG "-Wl,-rpath-link,")
  set(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "-Wl,-soname,")
  set(CMAKE_EXE_EXPORTS_C_FLAG "-Wl,--export-dynamic")
endif(EXISTS /usr/include/dlfcn.h)

include(Platform/UnixPaths)
