INCLUDE_DIRECTORIES(${Library_MODDIR})
ADD_LIBRARY(subdir_mods a.f90 b.f90)
ADD_EXECUTABLE(subdir_exe main.f90)
TARGET_LINK_LIBRARIES(subdir_exe subdir_mods)

# Test module output directory if available.
IF(CMAKE_Fortran_MODDIR_FLAG)
  SET_TARGET_PROPERTIES(subdir_mods PROPERTIES
    Fortran_MODULE_DIRECTORY modules
    )
ENDIF(CMAKE_Fortran_MODDIR_FLAG)
