cmake_minimum_required(VERSION 3.2.3)

project(GNUInstallDirsTest LANGUAGES NONE)

# ATTENTION Predefine CMAKE_SIZEOF_VOID_P, so `GNUInstallDirs` would be happy...
# The value itself doesn't matter, cuz contents of `LIBDIR' is not asserted,
# cuz may vari depending on distribution.
set(CMAKE_SIZEOF_VOID_P 8)
# Set prefix explicitly to the root of the filesystem
set(CMAKE_INSTALL_PREFIX "/usr/local")
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
set(expected_short_BINDIR "bin")
set(expected_long_BINDIR "/usr/local/bin")

set(expected_short_SBINDIR "sbin")
set(expected_long_SBINDIR "/usr/local/sbin")

set(expected_short_LIBEXECDIR "libexec")
set(expected_long_LIBEXECDIR "/usr/local/libexec")

set(expected_short_SYSCONFDIR "etc")
set(expected_long_SYSCONFDIR "/usr/local/etc")

set(expected_short_SHAREDSTATEDIR "com")
set(expected_long_SHAREDSTATEDIR "/usr/local/com")

set(expected_short_LOCALSTATEDIR "var")
set(expected_long_LOCALSTATEDIR "/usr/local/var")

set(expected_short_INCLUDEDIR "include")
set(expected_long_INCLUDEDIR "/usr/local/include")

set(expected_short_DATAROOTDIR "share")
set(expected_long_DATAROOTDIR "/usr/local/share")

set(expected_short_DATADIR "share")
set(expected_long_DATADIR "/usr/local/share")

set(expected_short_INFODIR "share/info")
set(expected_long_INFODIR "/usr/local/share/info")

set(expected_short_LOCALEDIR "share/locale")
set(expected_long_LOCALEDIR "/usr/local/share/locale")

set(expected_short_MANDIR "share/man")
set(expected_long_MANDIR "/usr/local/share/man")

set(expected_short_DOCDIR "share/doc/${PROJECT_NAME}")
set(expected_long_DOCDIR "/usr/local/share/doc/${PROJECT_NAME}")
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
