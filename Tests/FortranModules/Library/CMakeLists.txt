include_directories(${Library_MODDIR})
add_library(subdir_mods a.f90 b.f90)
add_executable(subdir_exe main.f90)
target_link_libraries(subdir_exe subdir_mods)

add_library(cycleA STATIC cycleA1.f90 cycleA2.f90)
add_library(cycleB STATIC cycleB1.f90 cycleB2.f90)
target_link_libraries(cycleA PRIVATE cycleB)
target_link_libraries(cycleB PRIVATE cycleA)

# Test module output directory if available.
if(CMAKE_Fortran_MODDIR_FLAG)
  set_target_properties(subdir_mods cycleA cycleB PROPERTIES
    Fortran_MODULE_DIRECTORY modules
    )
endif()
