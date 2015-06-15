cmake_minimum_required(VERSION 3.2.3)

project(GNUInstallDirsTest)

# Set prefix explicitly to the root of the filesystem
set(CMAKE_INSTALL_PREFIX "/opt/${PROJECT_NAME}")
include(GNUInstallDirs)

set(
    VAR_LIST
    BINDIR
    SBINDIR
    LIBEXECDIR
    SYSCONFDIR
    SHAREDSTATEDIR
    LOCALSTATEDIR
    INCLUDEDIR
    DATAROOTDIR
    DATADIR
    INFODIR
    LOCALEDIR
    MANDIR
    DOCDIR
  )

set(
    SHORT_DIR_LIST
    bin
    sbin
    libexec
    etc
    com
    var
    include
    share
    share
    share/info
    share/locale
    share/man
    share/doc/${PROJECT_NAME}
  )

set(
    LONG_DIR_LIST
    /opt/${PROJECT_NAME}/bin
    /opt/${PROJECT_NAME}/sbin
    /opt/${PROJECT_NAME}/libexec
    /etc/opt/${PROJECT_NAME}
    /opt/${PROJECT_NAME}/com
    /var/opt/${PROJECT_NAME}
    /opt/${PROJECT_NAME}/include
    /opt/${PROJECT_NAME}/share
    /opt/${PROJECT_NAME}/share
    /opt/${PROJECT_NAME}/share/info
    /opt/${PROJECT_NAME}/share/locale
    /opt/${PROJECT_NAME}/share/man
    /opt/${PROJECT_NAME}/share/doc/GNUInstallDirsTest
  )

foreach(dir ${VAR_LIST})
    list(FIND VAR_LIST ${dir} dir_pos)

    list(GET SHORT_DIR_LIST ${dir_pos} short_dir)
    list(GET LONG_DIR_LIST ${dir_pos} long_dir)

    # Checking short (relative) dir
    if(NOT CMAKE_INSTALL_${dir} STREQUAL short_dir)
        message(SEND_ERROR "'${CMAKE_INSTALL_${dir}}' != '${short_dir}'")
    endif()

    # Checking long (absolute) dir
    if(NOT CMAKE_INSTALL_FULL_${dir} STREQUAL long_dir)
        message(SEND_ERROR "'${CMAKE_INSTALL_FULL_${dir}}' != '${long_dir}'")
    endif()
endforeach()
