SET(CMAKE_SHARED_LIBRARY_PREFIX "lib")          # lib
SET(CMAKE_SHARED_LIBRARY_SUFFIX "..o")          # .so
SET(CMAKE_DL_LIBS "-lld")

IF(CMAKE_COMPILER_IS_GNUCXX) 
  SET(CMAKE_SHARED_LIBRARY_SUFFIX ".so")          # .so
  SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-shared -Wl,-G")       # -shared
  SET(CMAKE_SHARED_LIBRARY_LINK_FLAGS "-Wl,-brtl")         # +s, flag for exe link to use shared lib
ENDIF(CMAKE_COMPILER_IS_GNUCXX)
