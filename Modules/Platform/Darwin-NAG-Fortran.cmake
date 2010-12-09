set(CMAKE_Fortran_VERBOSE_FLAG "-Wl,-v") # Runs gcc under the hood.

# Need -fpp explicitly on case-insensitive filesystem.
set(CMAKE_Fortran_COMPILE_OBJECT
  "<CMAKE_Fortran_COMPILER> -fpp -o <OBJECT> <DEFINES> <FLAGS> -c <SOURCE>")
