SET(CMAKE_SHARED_LIBRARY_C_FLAGS "-K PIC")
SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-Wl,-Bexport")
# include the gcc flags 
INCLUDE(${CMAKE_ROOT}/Modules/Platform/gcc.cmake)