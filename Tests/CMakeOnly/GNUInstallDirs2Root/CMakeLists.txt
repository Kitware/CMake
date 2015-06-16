cmake_minimum_required(VERSION 3.2.3)

project(GNUInstallDirsTest LANGUAGES NONE)

# ATTENTION Predefine CMAKE_SIZEOF_VOID_P, so `GNUInstallDirs` would be happy...
# The value itself doesn't matter, cuz contents of `LIBDIR' is not asserted,
# cuz may vari depending on distribution.
set(CMAKE_SIZEOF_VOID_P 8)
# Set prefix explicitly to the root of the filesystem
set(CMAKE_INSTALL_PREFIX "/")
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

#BEGIN Expected values
set(expected_short_BINDIR "usr/bin")
set(expected_long_BINDIR "/usr/bin")

set(expected_short_SBINDIR "usr/sbin")
set(expected_long_SBINDIR "/usr/sbin")

set(expected_short_LIBEXECDIR "usr/libexec")
set(expected_long_LIBEXECDIR "/usr/libexec")

set(expected_short_SYSCONFDIR "etc")
set(expected_long_SYSCONFDIR "/etc")

set(expected_short_SHAREDSTATEDIR "usr/com")
set(expected_long_SHAREDSTATEDIR "/usr/com")

set(expected_short_LOCALSTATEDIR "var")
set(expected_long_LOCALSTATEDIR "/var")

set(expected_short_INCLUDEDIR "usr/include")
set(expected_long_INCLUDEDIR "/usr/include")

set(expected_short_DATAROOTDIR "usr/share")
set(expected_long_DATAROOTDIR "/usr/share")

set(expected_short_DATADIR "usr/share")
set(expected_long_DATADIR "/usr/share")

set(expected_short_INFODIR "usr/share/info")
set(expected_long_INFODIR "/usr/share/info")

set(expected_short_LOCALEDIR "usr/share/locale")
set(expected_long_LOCALEDIR "/usr/share/locale")

set(expected_short_MANDIR "usr/share/man")
set(expected_long_MANDIR "/usr/share/man")

set(expected_short_DOCDIR "usr/share/doc/${PROJECT_NAME}")
set(expected_long_DOCDIR "/usr/share/doc/${PROJECT_NAME}")
#END Expected values

foreach(dir ${VAR_LIST})
    # Checking short (relative) dir
    if(NOT "${CMAKE_INSTALL_${dir}}" STREQUAL "${expected_short_${dir}}")
        message(SEND_ERROR "'${CMAKE_INSTALL_${dir}}' != '${expected_short_${dir}}'")
    endif()

    # Checking long (absolute) dir
    if(NOT "${CMAKE_INSTALL_FULL_${dir}}" STREQUAL "${expected_long_${dir}}")
        message(SEND_ERROR "'${CMAKE_INSTALL_FULL_${dir}}' != '${expected_long_${dir}}'")
    endif()
endforeach()
