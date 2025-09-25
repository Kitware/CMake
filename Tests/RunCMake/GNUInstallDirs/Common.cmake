set(CMAKE_SIZEOF_VOID_P 8)
set(CMAKE_LIBRARY_ARCHITECTURE "arch")
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
  RUNSTATEDIR
  MANDIR
  SBINDIR
  SHAREDSTATEDIR
  SYSCONFDIR
  )
set(dependent_dirs
  DATADIR
  LOCALEDIR
  DOCDIR
  RUNSTATEDIR
)
# See special cases for these in GNUInstallDirs
if(NOT CMAKE_SYSTEM_NAME MATCHES "^(([^kF].*)?BSD|DragonFly)$")
  list(APPEND dependent_dirs INFODIR)
endif()
if(NOT CMAKE_SYSTEM_NAME MATCHES "^(([^k].*)?BSD|DragonFly)$"
    OR CMAKE_SYSTEM_NAME MATCHES "^(FreeBSD)$")
  list(APPEND dependent_dirs MANDIR)
endif()

foreach(dir ${dirs})
  if(dir IN_LIST dependent_dirs)
    message("CMAKE_INSTALL_${dir}='${CMAKE_INSTALL_${dir}}'")
  else()
    message("CMAKE_INSTALL_${dir}='$CACHE{CMAKE_INSTALL_${dir}}'")
  endif()
endforeach()
foreach(dir ${dirs})
  message("CMAKE_INSTALL_FULL_${dir}='${CMAKE_INSTALL_FULL_${dir}}'")
endforeach()
