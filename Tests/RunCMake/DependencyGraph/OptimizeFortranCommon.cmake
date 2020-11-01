enable_language(C)
enable_language(Fortran)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY out)

add_library(FortranTop STATIC mylib.f90)
add_library(CMiddle STATIC mylib.c)
add_library(FortranBottom STATIC mylib.f90)

target_link_libraries(FortranTop PRIVATE CMiddle)
target_link_libraries(CMiddle PRIVATE FortranBottom)

if(OPTIMIZE_TOP)
  set_target_properties(FortranTop PROPERTIES
    OPTIMIZE_DEPENDENCIES TRUE)
endif()
if(OPTIMIZE_MIDDLE)
  set_target_properties(CMiddle PROPERTIES
    OPTIMIZE_DEPENDENCIES TRUE)
endif()

include(WriteTargets.cmake)
write_targets()
