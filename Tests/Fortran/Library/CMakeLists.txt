ADD_LIBRARY(subdir_mods a.f90 b.f90)
ADD_EXECUTABLE(subdir_exe main.f90)
TARGET_LINK_LIBRARIES(subdir_exe subdir_mods)
