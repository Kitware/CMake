set(CMAKE_SIZEOF_VOID_P 8)
set(CMAKE_LIBRARY_ARCHITECTURE "arch")
set(CMAKE_INSTALL_PREFIX "/usr/local")
include(GNUInstallDirs)
set(dirs
  BINDIR
  INCLUDEDIR
  LIBDIR
  )
foreach(dir ${dirs})
  message(STATUS "CMAKE_INSTALL_${dir}='$CACHE{CMAKE_INSTALL_${dir}}'")
endforeach()
foreach(dir ${dirs})
  message(STATUS "CMAKE_INSTALL_FULL_${dir}='${CMAKE_INSTALL_FULL_${dir}}'")
endforeach()
