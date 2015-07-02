# test /bigobj for msvc
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()
add_library(autoexport2 SHARED sub.cxx)
