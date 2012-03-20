if("${CMAKE_GENERATOR}" MATCHES "Visual Studio 6")
  # VS 6 generator does not use per-target object locations.
  set(vs6 _vs6)
endif()

# Add -fPIC so objects can be used in shared libraries.
# TODO: Need property for this.
if(CMAKE_SHARED_LIBRARY_C_FLAGS AND NOT WATCOM)
  set(CMAKE_C_FLAGS "${CMAKE_SHARED_LIBRARY_C_FLAGS} ${CMAKE_C_FLAGS}")
endif()

add_definitions(-DB_DEF)
add_library(B OBJECT b1.c b2.c)
add_library(Bexport OBJECT b1${vs6}.c b2${vs6}.c)
set_property(TARGET Bexport PROPERTY COMPILE_DEFINITIONS Bexport)
