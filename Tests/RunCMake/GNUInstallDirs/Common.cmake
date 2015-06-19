set(CMAKE_SIZEOF_VOID_P 8)
set(CMAKE_LIBRARY_ARCHITECTURE "arch")
if(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
  set(CMAKE_SYSTEM_NAME "OpenBSD-Fake")
endif()
include(GNUInstallDirs)
set(dirs
  BINDIR
  DATADIR
  DATAROOTDIR
  DOCDIR
  INCLUDEDIR
  INFODIR
  LIBDIR
  LIBEXECDIR
  LOCALEDIR
  LOCALSTATEDIR
  MANDIR
  SBINDIR
  SHAREDSTATEDIR
  SYSCONFDIR
  )
foreach(dir ${dirs})
  message("CMAKE_INSTALL_${dir}='${CMAKE_INSTALL_${dir}}'")
endforeach()
foreach(dir ${dirs})
  message("CMAKE_INSTALL_FULL_${dir}='${CMAKE_INSTALL_FULL_${dir}}'")
endforeach()
