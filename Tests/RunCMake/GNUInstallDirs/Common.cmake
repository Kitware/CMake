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
